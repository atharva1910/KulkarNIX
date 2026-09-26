use common::address::VirtualAddress;
use core::{fmt::Write, ptr::null_mut};
use crate::SPrint;

pub struct List {
    next: *mut List,
    prev: *mut List,
}

impl List {
    pub fn iter(&self) -> ListIter {
        ListIter {
            current: self.next,
        }
    }

    pub fn is_empty(&self) -> bool {
        self.next == null_mut()
    }

    pub fn create_node(addr: VirtualAddress) -> *mut List {
        let node = *addr as *mut List;
        if let Some(ref_node) = unsafe{node.as_mut()} {
            ref_node.next = core::ptr::null_mut();
            ref_node.prev = core::ptr::null_mut();
        }

        node
    }

    pub fn insert(&mut self, node: &mut List) {
        if self.next == null_mut() {
            self.next = node;
            self.prev = node;
        } else {
            unsafe {
                (*self.prev).next = node;
            }
            self.prev = node;
        }
    }

    pub fn remove(&mut self, node: &mut List) {
        let next_node = node.next;
        let prev_node = node.prev;
        unsafe {
            (*prev_node).next = next_node;
            (*next_node).prev = prev_node;
        }
    }
}

pub struct ListIter {
    current: *mut List,
}

impl Iterator for ListIter {
    type Item = *mut List;
    fn next(&mut self) -> Option<Self::Item> {
        let Some(curr) = (unsafe{self.current.as_ref()}) else {
            return None;
        };
        let ret = self.current;
        self.current =  curr.next;
        Some(ret)
    }
}
