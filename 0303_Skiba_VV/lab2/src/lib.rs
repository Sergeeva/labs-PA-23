mod orig_task;
pub use orig_task::*;

mod my_basic_queue;
pub use my_basic_queue::*;
mod my_shared_queue;
pub use my_shared_queue::*;

mod task_runner;
pub use task_runner::*;


#[derive(Debug)]
pub struct TaskPoolCounters {
    consumers_tasks_remaining: usize,
    producers_tasks_remaining: usize,
}

impl TaskPoolCounters {
    pub fn new(tasks_c: usize) -> Self {
        Self {
            consumers_tasks_remaining: tasks_c,
            producers_tasks_remaining: tasks_c,
        }
    }
    pub fn on_add_task(&mut self) {
        self.producers_tasks_remaining -= 1;
    }
    pub fn on_take_task(&mut self) {
        self.consumers_tasks_remaining -= 1;
    }

    pub fn was_last_producer(&self) -> bool {
        self.producers_tasks_remaining == 0
    }
    pub fn has_planned_tasks(&self) -> bool {
        self.consumers_tasks_remaining == 0
    }
}