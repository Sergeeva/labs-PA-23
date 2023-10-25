use std::{sync::{Arc, Mutex, MutexGuard}, cell::RefCell, mem};
use std::sync::Condvar;
use crate::{input_square_matrices, Mat, multiply_matrices, Runner, SharedDataBuilder, TaskPoolCounters};


#[derive(Debug)]
pub struct MySharedQueue<T> {
    head: Mutex<MySharedQueueLink<T>>,
    tail: Mutex<MySharedQueueLink<T>>
}

pub type MySharedQueueLink<T> = Option<Arc<Mutex<SharedQueueNode<T>>>>;


#[derive(Debug)]
pub struct SharedQueueNode<T> {
    val: T,
    next: MySharedQueueLink<T>
}

impl<T> SharedQueueNode<T> {
    pub fn new(val: T) -> Self {
        Self {
            val,
            next: None
        }
    }
}


impl<T> MySharedQueue<T> {
    pub const fn new() -> Self {
        Self {
            head: Mutex::new(None),
            tail: Mutex::new(None)
        }
    }
}

impl<T> MySharedQueue<T> {
    pub fn push(&self, val: T) {

        let new_tail = Arc::new(Mutex::new(SharedQueueNode::new(val)));

        let self_tail = &mut *self.tail.lock().unwrap();
        let old_tail = mem::replace(self_tail, Some(new_tail.clone()));
        let _ = self_tail;
        if let Some(n) = old_tail {
            n.lock().unwrap().next = Some(new_tail);
        }
        else {
            let self_head = &mut *self.head.lock().unwrap();
            *self_head = Some(new_tail.clone());
        }
    }

    pub fn pop(&self) -> Option<T> {
        let self_head = &mut *self.head.lock().unwrap();

        let old_head = self_head.take();
        if let Some(n) = old_head {
            *self_head = n.lock().unwrap().next.take();
            if self_head.is_none() {
                let self_tail = &mut *self.tail.lock().unwrap();
                *self_tail = None;
            }
            let _ = self_head;
            let val = Arc::try_unwrap(n).ok().unwrap().into_inner().unwrap().val;
            Some(val)
        }
        else {
            None
        }
    }

    pub fn is_empty(&self) -> bool {
        self.head.lock().unwrap().is_none()
    }
}


pub struct SharedQueueRunner;
#[derive(Debug)]
pub struct SharedQueueSharedData<T> {
    task_pool_counters: Mutex<TaskPoolCounters>,
    consumer_waiter: Condvar,
    shared_queue: MySharedQueue<T>,
}

impl<T> SharedDataBuilder for SharedQueueSharedData<T> {
    fn new(producers_c: usize, consumers_c: usize, matrix_size: usize) -> Self {
        SharedQueueSharedData {
            task_pool_counters: Mutex::new(TaskPoolCounters::new(producers_c)),
            consumer_waiter: Condvar::new(),
            shared_queue: MySharedQueue::new()
        }
    }
}

impl Runner for SharedQueueRunner {
    type Msg = (Mat, Mat);
    type SharedData = SharedQueueSharedData<Self::Msg>;
    fn run_producer(mat_size: usize, shared_data: Arc<Self::SharedData>) {
        let rand_mat = input_square_matrices(mat_size); // payload outside
        let mut guard = shared_data.task_pool_counters.lock().unwrap();
        shared_data.shared_queue.push(rand_mat);
        guard.on_add_task();
        shared_data.consumer_waiter.notify_one();
        if guard.was_last_producer() {
            // println!("Notifying all consumers...");
            shared_data.consumer_waiter.notify_all();
        }
        drop(guard);
        // TASKS_PREPARED.fetch_add(1, std::sync::atomic::Ordering::SeqCst);
        // println!("Producer {} finished 1 task", i);
    }
    fn run_consumer(shared_data: Arc<Self::SharedData>) {
        let mut guard = shared_data.task_pool_counters.lock().unwrap();
        while !guard.has_planned_tasks() { //exit condition, help to finish thread when there is no work to do
            if shared_data.shared_queue.is_empty() {
                // println!("\tConsumer {} is waiting...", i);
                guard = shared_data.consumer_waiter.wait(guard).unwrap();
                // println!("\tConsumer {} has finished wait", i)
            }
            else {
                let inp_data = shared_data.shared_queue.pop().unwrap();
                guard.on_take_task();

                drop(guard);
                // here data was taken from queue
                // TASKS_ACQUIRED.fetch_add(1, std::sync::atomic::Ordering::SeqCst);
                // println!("\tConsumer {} got data", i);
                let res = multiply_matrices(inp_data).unwrap(); // payload outside
                // let filename = format!("res_{}_{}.txt", i, producers_c);
                // res.save_to_file(&filename);
                guard = shared_data.task_pool_counters.lock().unwrap();
            }

        }
        // println!("\tConsumer {} has exited main loop!", i)
    }
}