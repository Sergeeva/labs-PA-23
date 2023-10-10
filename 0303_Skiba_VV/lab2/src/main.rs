use std::cell::RefCell;
use std::error::Error;
use std::io::{stdin, stdout, Write};
use std::sync::atomic::AtomicUsize;
use std::sync::{Mutex, Arc, Condvar};
use std::{thread, mem};
use std::time::{Duration, Instant};

use lab2::{BlockingQueue, BaseQueue, QueueTrait, PartialLockQueue, QueueTraitLocal, input_square_matrices, multiply_matrices, MatExt};

static TASKS_ACQUIRED: AtomicUsize = AtomicUsize::new(0);
static TASKS_PREPARED: AtomicUsize = AtomicUsize::new(0);

fn main() -> anyhow::Result<()> {
    // println!("\n\t*** LAB2 ***");
    print!(" > Enter producers count: ");
    stdout().flush()?;


    let mut producers_c = String::new();
    stdin().read_line(&mut producers_c)?;
    let producers_c: usize = producers_c.trim().parse()?;

    print!(" > Enter consumers count: ");
    stdout().flush()?;

    let mut consumers_c = String::new();
    stdin().read_line(&mut consumers_c)?;
    let consumers_c: usize = consumers_c.trim().parse()?;

    print!(" > Enter matrix size: ");
    stdout().flush()?;

    let mut matrix_size = String::new();
    stdin().read_line(&mut matrix_size)?;
    let matrix_size: usize = matrix_size.trim().parse()?;

    // let q = BlockingQueue::new();
    // q.push(123);
    // q.push(321);
    // println!("pop {:?}", q.pop());
    // q.push(555);
    //
    // println!("pop {:?}", q.pop());
    // println!("pop {:?}", q.pop());
    // println!("pop {:?}", q.pop());
    //
    // let q_clone = q.clone();
    // thread::spawn(move|| {
    //     q_clone.push(123);
    // }).join().unwrap();
    //
    // println!("pop {:?}", q.pop());
    //
    // let p_q = Arc::new(PartialBlockQueue::new());
    // p_q.push(123);
    // p_q.push(321);
    // println!("pop {:?}", p_q.pop());
    // p_q.push(555);
    //
    // println!("pop {:?}", p_q.pop());
    // println!("pop {:?}", p_q.pop());
    // println!("pop {:?}", p_q.pop());
    //
    // let p_q_clone = p_q.clone();
    // thread::spawn(move|| {
    //     p_q_clone.push(123);
    // }).join().unwrap();
    //
    // println!("pop {:?}", p_q.pop());
   
    let mut join_handles = vec![];

    let start = Instant::now();
    #[cfg(feature="condvar")]
    {
        let q = Arc::new((Mutex::new(BlockingQueue::new()), Condvar::new()));

        for i in 0..producers_c {
            let q_copy = q.clone();
            join_handles.push(thread::spawn(move || {
                let rand_mat = input_square_matrices(matrix_size);
                q_copy.0.lock().unwrap().push(rand_mat);
                TASKS_PREPARED.fetch_add(1, std::sync::atomic::Ordering::SeqCst);
                if TASKS_PREPARED.load(std::sync::atomic::Ordering::SeqCst) == producers_c {
                    println!("Producer {} finished last task. Notifying all consumers", i);
                    q_copy.1.notify_all();
                }
                else {
                    println!("Producer {} finished 1 task", i);
                    q_copy.1.notify_one();
                }
            }));
        }

        for i in 0..consumers_c {
            let q_copy = q.clone();
            join_handles.push(thread::spawn(move || {
                while TASKS_ACQUIRED.load(std::sync::atomic::Ordering::SeqCst) != producers_c {
                    let mut guard = q_copy.0.lock().unwrap();

                    println!("Consumer {} waiting", i);
                    match q_copy.1.wait_timeout(guard, Duration::from_millis(10)).unwrap() {
                        (new_guard, timeout) => {
                            guard = new_guard;
                            if timeout.timed_out() {
                                println!("Consumer {} timed out", i);
                            }
                            else {
                                println!("Consumer {} got notified", i);
                            }
                        }
                    }
                    println!("Consumer {} got notified", i);

                    let inp_data = guard.pop();
                    mem::drop(guard);
                    if let Some(inp_data) = inp_data {
                        TASKS_ACQUIRED.fetch_add(1, std::sync::atomic::Ordering::SeqCst);
                        println!("Consumer {} got data", i);
                        let res = multiply_matrices(inp_data).unwrap();
                        // let filename = format!("res_{}_{}.txt", i, producers_c);
                        // res.save_to_file(&filename);
                    }
                    else {
                        println!("Consumer {} got None", i);
                    }
                }
                println!("Consumer {} finished", i);
            }));
        }
    }


    #[cfg(feature="fulllock")]
    {
        let q = Arc::new(BlockingQueue::new());

        for i in 0..producers_c {
            let q_copy = q.clone();
            join_handles.push(thread::spawn(move || {
                let rand_mat = input_square_matrices(matrix_size);
                q_copy.push(rand_mat);
                TASKS_PREPARED.fetch_add(1, std::sync::atomic::Ordering::SeqCst);
                println!("Producer {} finished 1 task", i);
            }));
        }

        for i in 0..consumers_c {
            let q_copy = q.clone();
            join_handles.push(thread::spawn(move || {
                while TASKS_ACQUIRED.load(std::sync::atomic::Ordering::SeqCst) != producers_c {
                    let inp_data = q_copy.pop();
                    if let Some(inp_data) = inp_data {
                        // here data was taken from queue
                        TASKS_ACQUIRED.fetch_add(1, std::sync::atomic::Ordering::SeqCst);
                        println!("Consumer {} got data", i);
                        let res = multiply_matrices(inp_data).unwrap();
                        // let filename = format!("res_{}_{}.txt", i, producers_c);
                        // res.save_to_file(&filename);
                    }
                    else {
                        println!("Consumer {} got None", i);
                    }
                }
            }));
        }
    }

    #[cfg(feature = "partiallock")]
    {
        let q = Arc::new(PartialLockQueue::new());

        for i in 0..producers_c {
            let q_copy = q.clone();
            join_handles.push(thread::spawn(move || {
                let rand_mat = input_square_matrices(matrix_size);
                q_copy.push(rand_mat);
                TASKS_PREPARED.fetch_add(1, std::sync::atomic::Ordering::SeqCst);
                println!("Producer {} finished 1 task", i);
            }));
        }

        for i in 0..consumers_c {
            let q_copy = q.clone();
            join_handles.push(thread::spawn(move || {
                while TASKS_ACQUIRED.load(std::sync::atomic::Ordering::SeqCst) != producers_c {
                    let inp_data = q_copy.pop();
                    if let Some(inp_data) = inp_data {
                        // here data was taken from queue
                        TASKS_ACQUIRED.fetch_add(1, std::sync::atomic::Ordering::SeqCst);
                        println!("Consumer {} got data", i);
                        let res = multiply_matrices(inp_data).unwrap();
                        // let filename = format!("res_{}_{}.txt", i, producers_c);
                        // res.save_to_file(&filename);
                    }
                    else {
                        println!("Consumer {} got None", i);
                    }
                }
            }));
        }
    }


    for handle in join_handles {
        handle.join().unwrap();
    }
    let duration = start.elapsed();
    println!("Time elapsed is: {:?}", duration);

    println!("{} results recieved", TASKS_ACQUIRED.load(std::sync::atomic::Ordering::SeqCst));
    println!("{} results prepared", TASKS_PREPARED.load(std::sync::atomic::Ordering::SeqCst));


    Ok(())
}
