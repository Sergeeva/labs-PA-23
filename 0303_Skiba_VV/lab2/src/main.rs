use std::cell::RefCell;
use std::error::Error;
use std::io::{stdin, stdout, Write};
use std::sync::atomic::AtomicUsize;
use std::sync::{Mutex, Arc, Condvar};
use std::{thread, mem};
use std::thread::JoinHandle;
use std::time::{Duration, Instant};

use crossbeam::queue::SegQueue;

use lab2::{BasicQueue, MySharedQueue, input_square_matrices, multiply_matrices, MatExt, TaskPoolCounters, run_tasks, BlockingQueueRunner, SharedQueueRunner};


fn main() -> anyhow::Result<()> {
    // println!("\n\t*** LAB2 ***");
    print!(" > Enter producers count: ");
    stdout().flush()?;


    // let mut producers_c = String::new();
    // stdin().read_line(&mut producers_c)?;
    // let producers_c: usize = producers_c.trim().parse()?;
    let producers_c = 10000;

    print!(" > Enter consumers count: ");
    stdout().flush()?;

    // let mut consumers_c = String::new();
    // stdin().read_line(&mut consumers_c)?;
    // let consumers_c: usize = consumers_c.trim().parse()?;
    let consumers_c = 10000;

    print!(" > Enter matrix size: ");
    stdout().flush()?;

    // let mut matrix_size = String::new();
    // stdin().read_line(&mut matrix_size)?;
    // let matrix_size: usize = matrix_size.trim().parse()?;
    let matrix_size = 1;


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

    let dur = run_tasks::<SharedQueueRunner>(consumers_c, producers_c, matrix_size);
    println!("Executing all tasks took {:?}!", dur);

    let dur = run_tasks::<BlockingQueueRunner>(consumers_c, producers_c, matrix_size);
    println!("Executing all tasks took {:?}!", dur);

    Ok(())
}
