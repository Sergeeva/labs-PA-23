use std::sync::atomic::{AtomicUsize, Ordering};
use std::sync::{Arc, Condvar, Mutex};
use std::thread;
use std::thread::yield_now;
use std::time::{Duration, Instant};
use crossbeam::sync::Parker;
use crate::{BasicQueue, input_square_matrices, Mat, multiply_matrices, MySharedQueue, TaskPoolCounters};


pub static TASKS_PREPARED: AtomicUsize = AtomicUsize::new(0);
pub static TASKS_SOLVED: AtomicUsize = AtomicUsize::new(0);
pub static TASKS_TOTAL: AtomicUsize = AtomicUsize::new(0);

pub fn run_tasks<R: Runner>(tasks_c: usize, workers_c: usize, mat_size: usize) -> Duration {
    let mut join_handles = Vec::new();

    TASKS_PREPARED.store(0, Ordering::SeqCst);
    TASKS_SOLVED.store(0, Ordering::SeqCst);
    TASKS_TOTAL.store(tasks_c, Ordering::SeqCst);

    let start = Instant::now();
    let worker_shared_data = Arc::new(R::SharedData::new(tasks_c, workers_c, mat_size));

    for i in 0..tasks_c {
        let worker_shared_data_ref = worker_shared_data.clone();
        join_handles.push(thread::spawn(move || {
            R::run_producer(mat_size, worker_shared_data_ref);
            TASKS_PREPARED.fetch_add(1, Ordering::SeqCst);
        }));
    }

    for i in 0..workers_c {
        let worker_shared_data_ref = worker_shared_data.clone();
        join_handles.push(thread::spawn(move || {
            R::run_consumer(worker_shared_data_ref);
        }));
    }

    for handle in join_handles {
        handle.join().unwrap();
    }
    assert_eq!(TASKS_PREPARED.load(Ordering::SeqCst), tasks_c);
    assert_eq!(TASKS_SOLVED.load(Ordering::SeqCst), tasks_c);
    let duration = start.elapsed();
    duration
}

pub trait Runner {
    type SharedData: SharedDataBuilder + Send + Sync + 'static;
    type Msg;
    fn run_producer(mat_size: usize, shared_data: Arc<Self::SharedData>);
    fn run_consumer(shared_data: Arc<Self::SharedData>);
}

pub trait SharedDataBuilder {
    fn new(producers_c: usize, consumers_c: usize, matrix_size: usize) -> Self;
}
