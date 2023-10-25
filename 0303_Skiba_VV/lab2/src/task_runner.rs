use std::sync::atomic::AtomicUsize;
use std::sync::{Arc, Condvar, Mutex};
use std::thread;
use std::time::{Duration, Instant};
use crate::{BasicQueue, input_square_matrices, Mat, multiply_matrices, MySharedQueue, TaskPoolCounters};


static TASKS_ACQUIRED: AtomicUsize = AtomicUsize::new(0);
static TASKS_PREPARED: AtomicUsize = AtomicUsize::new(0);

pub fn run_tasks<R: Runner>(tasks_c: usize, workers_c: usize, mat_size: usize) -> Duration {
    let mut join_handles = Vec::new();

    let start = Instant::now();
    let worker_shared_data = Arc::new(R::SharedData::new(tasks_c, workers_c, mat_size));

    for i in 0..tasks_c {
        let worker_shared_data_ref = worker_shared_data.clone();
        join_handles.push(thread::spawn(move || {
            R::run_producer(mat_size, worker_shared_data_ref);
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
