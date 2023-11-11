use std::{sync::Arc, mem};
use crossbeam::queue::SegQueue;
use crate::{input_square_matrices, Mat, multiply_matrices, Runner, SharedDataBuilder, SharedQueue, TaskPoolCounters};

use crate::custom_sync_primitieves::*;

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


impl<T> SharedQueue for MySharedQueue<T> {
    type Item = T;

    fn new() -> Self {
        Self {
            head: Mutex::new(None),
            tail: Mutex::new(None)
        }
    }
    fn push(&self, val: T) {

        let new_tail = Arc::new(Mutex::new(SharedQueueNode::new(val)));

        let self_tail = &mut *self.tail.mylock();
        let old_tail = mem::replace(self_tail, Some(new_tail.clone()));
        let _ = self_tail;
        if let Some(n) = old_tail {
            n.mylock().next = Some(new_tail);
        }
        else {
            let self_head = &mut *self.head.mylock();
            *self_head = Some(new_tail.clone());
        }
    }

    fn pop(&self) -> Option<T> {
        let self_head = &mut *self.head.mylock();

        let old_head = self_head.take();
        if let Some(n) = old_head {
            *self_head = n.mylock().next.take();
            if self_head.is_none() {
                let self_tail = &mut *self.tail.mylock();
                *self_tail = None;
            }
            let _ = self_head;
                let val = Arc::try_unwrap(n).ok().unwrap().my_into_inner().val;
            Some(val)
        }
        else {
            None
        }
    }

    fn is_empty(&self) -> bool {
        self.head.mylock().is_none()
    }
}

impl<T> Drop for MySharedQueue<T> {
    fn drop(&mut self) {
        while self.pop().is_some() {}
    }
}