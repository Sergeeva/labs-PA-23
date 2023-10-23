use std::sync::{Arc, Mutex};


use crate::BaseQueue;
use crate::QueueTrait;


pub struct BlockingQueue<T> {
    queue: Mutex<BaseQueue<T>>
}

impl<T> BlockingQueue<T> {
    pub fn new() -> Self {
        Self {
            queue: Mutex::new(BaseQueue::new())
        }
    }
    pub fn push(&self, val: T) {
        self.queue.lock().unwrap().push(val);
    }
    pub fn pop(&self) -> Option<T> {
        self.queue.lock().unwrap().pop()
    }
}

impl<T> Default for BlockingQueue<T> {
    fn default() -> Self {
        Self::new()
    }
}

unsafe impl<T> Send for BlockingQueue<T> {}
unsafe impl<T> Sync for BlockingQueue<T> {}
