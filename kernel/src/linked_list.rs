use common::address::VirtualAddress;
use core::{fmt::Write, ptr::null_mut};
use crate::SPrint;

pub struct RawList {
    next: *mut RawList,
    prev: *mut RawList,
}

impl Iterator for &RawList {
    type Item = *mut RawList;
    fn next(&mut self) -> Option<Self::Item> {
        if self.next == null_mut() {
            return None;
        }
        Some(self.next)
    }
}

impl RawList {
    pub fn new() -> Self {
        Self {
            next: core::ptr::null_mut(),
            prev: core::ptr::null_mut(),
        }
    }

    pub fn is_empty(&self) -> bool {
        self.next == core::ptr::null_mut() &&
            self.prev == core::ptr::null_mut()
    }

    pub fn remove(&mut self) {
        let next_node = self.next;
        let prev_node = self.prev;
        unsafe {
            (*prev_node).next = next_node;
            (*next_node).prev = prev_node;
        }
    }

    pub fn init(x: *mut RawList) -> *mut RawList {
        unsafe {
            (*x).next = core::ptr::null_mut();
            (*x).prev = core::ptr::null_mut();
        }
        x
    }

    pub fn insert(&mut self, node: *mut RawList) {
        unsafe {
            if self.is_empty() {
                self.next = node;
                self.prev = node;
                (*node).prev = null_mut();
                (*node).next = null_mut();
            } else {
                let tail = (*node).prev;
                (*tail).next = node;
                (*node).prev = tail;
                (*node).next = null_mut();
                self.prev = node;
            }
        }
    }

    pub fn create_node(addr: VirtualAddress) -> *mut RawList {
        let node = addr.get_raw() as *mut RawList;
        unsafe {
            (*node).next = core::ptr::null_mut();
            (*node).prev = core::ptr::null_mut();
        }

        node
    }
}
