use std::{sync::{Arc, Mutex}, cell::RefCell, mem};


pub struct BaseQueue<T> {
    head: Link<T>,
    tail: Link<T>
}

pub struct Node<T> {
    val: T,
    next: Link<T>
}

impl<T> Node<T> {
    fn new(val: T) -> Self {
        Self {
            val,
            next: None
        }
    }
}

pub type Link<T> = Option<Arc<RefCell<Node<T>>>>;

pub trait QueueTrait<T> {
    fn push(&mut self, val: T);
    fn pop(&mut self) -> Option<T>;
}

impl<T> BaseQueue<T> {
    pub const fn new() -> Self {
        Self {
            head: None,
            tail: None
        }
    }
}

impl<T> QueueTrait<T> for BaseQueue<T> {
    fn pop(&mut self) -> Option<T> {
        let old_head = self.head.take();
        if let Some(n) = old_head {
            self.head = n.borrow_mut().next.take();
            if self.head.is_none() {
                self.tail = None;
            }
            let val = Arc::try_unwrap(n).ok().unwrap().into_inner().val;
            Some(val)
        }
        else {
            None
        }
    }
    fn push(&mut self, val: T) {
        let new_tail = Arc::new(RefCell::new(Node::new(val)));
        let old_tail = mem::replace(&mut self.tail, Some(new_tail.clone()));
        if let Some(n) = old_tail {
            n.borrow_mut().next = Some(new_tail);
        }
        else {
            self.head = Some(new_tail);
        }
    }
}

