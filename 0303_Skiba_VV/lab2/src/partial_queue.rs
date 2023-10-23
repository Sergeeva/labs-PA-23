use std::{sync::{Arc, Mutex, MutexGuard}, cell::RefCell, mem};

use crate::QueueTrait;


// immutable access
pub struct PartialLockQueue<T> {
    head: Mutex<PartialQueueLink<T>>,
    tail: Mutex<PartialQueueLink<T>>
}

pub type PartialQueueLink<T> = Option<Arc<Mutex<PartialNode<T>>>>;

pub trait QueueTraitLocal<T> {
    fn push(&self, val: T);
    fn pop(&self) -> Option<T>;
}

// mutable access after mutex lock
pub struct PartialNode<T> {
    val: T,
    next: PartialQueueLink<T>
}

impl<T> PartialNode<T> {
    pub fn new(val: T) -> Self {
        Self {
            val,
            next: None
        }
    }
}


impl<T> PartialLockQueue<T> {
    pub const fn new() -> Self {
        Self {
            head: Mutex::new(None),
            tail: Mutex::new(None)
        }
    }
}

impl<T> QueueTraitLocal<T> for PartialLockQueue<T> {
    fn push(&self, val: T) {

        let new_tail = Arc::new(Mutex::new(PartialNode::new(val)));

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

    fn pop(&self) -> Option<T> {
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
}

