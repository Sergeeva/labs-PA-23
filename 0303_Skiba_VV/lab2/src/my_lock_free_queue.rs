use std::sync::atomic::{AtomicPtr, Ordering};
use std::ptr;
use crate::SharedQueue;

struct LockFreeQueueNode<T> {
    data: T,
    next: AtomicPtr<LockFreeQueueNode<T>>,
}

pub struct MyLockFreeQueue<T> {
    head: AtomicPtr<LockFreeQueueNode<T>>,
    tail: AtomicPtr<LockFreeQueueNode<T>>,
}


impl<T> SharedQueue for MyLockFreeQueue<T> {
    type Item = T;

    fn new() -> Self {
        Self {
            head: AtomicPtr::new(ptr::null_mut()),
            tail: AtomicPtr::new(ptr::null_mut()),
        }
    }
    fn push(&self, data: T) {
        // println!("lockfree push start");
        let new_node = Box::into_raw(Box::new(LockFreeQueueNode {
            data,
            next: AtomicPtr::new(ptr::null_mut()),
        }));

        let mut tail;
        loop {
            // println!("lockfree push start");
            tail = self.tail.load(Ordering::Relaxed);
            if tail.is_null() {
                if self.head.compare_exchange(tail, new_node, Ordering::AcqRel, Ordering::Acquire).is_ok() {
                    self.tail.store(new_node, Ordering::Release);
                    // println!("lockfree push end");
                    return;
                }
            } else {
                if self.tail.compare_exchange(tail, new_node, Ordering::AcqRel, Ordering::Acquire).is_ok() {
                    unsafe { (*tail).next.store(new_node, Ordering::Release) };
                    // println!("lockfree push end");
                    return;
                }
            }
        }
    }

    fn pop(&self) -> Option<T> {
        // println!("lockfree pop start");
        loop {
            let head = self.head.load(Ordering::Acquire);
            if head.is_null() {
                // println!("lockfree pop end");
                return None;
            } else {
                let next = unsafe { (*head).next.load(Ordering::Acquire) };
                if self.head.compare_exchange(head, next, Ordering::AcqRel, Ordering::Acquire).is_ok() {
                    unsafe {
                        let node = Box::from_raw(head);
                        // println!("lockfree pop end");
                        return Some(node.data);
                    }
                }
            }
        }
    }

    fn is_empty(&self) -> bool {

        self.head.load(Ordering::SeqCst).is_null()
    }
}