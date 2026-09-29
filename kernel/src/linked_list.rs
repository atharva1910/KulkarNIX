use common::address::VirtualAddress;
use core::{fmt::Write, ptr::null_mut};
use crate::SPrint;

pub struct List {
    next: *mut List,
    prev: *mut List,
}

impl List {
    pub fn node_mut(&self, node: *mut List) -> Option<&mut List> {
        unsafe {
            node.as_mut()
        }
    }

    pub fn iter(&mut self) -> ListIter {
        ListIter {
            current: self,
            next: self,
        }
    }

    pub fn get_next(&self) -> *mut List {
        self.next
    }

    pub fn is_empty(&self) -> bool {
        self.next == null_mut()
    }

    pub fn init(&mut self) {
        self.next = null_mut();
        self.prev = null_mut();
    }

    pub fn create_node(addr: VirtualAddress) -> *mut List {
        let node = *addr as *mut List;
        if let Some(ref_node) = unsafe{node.as_mut()} {
            ref_node.next = core::ptr::null_mut();
            ref_node.prev = core::ptr::null_mut();
        }

        node
    }

    pub fn insert_before(&mut self, after: &mut List, node: &mut List) {
        let before = after.prev;

        node.next = after;
        after.prev = node;

        if let Some(before) = unsafe{before.as_mut()} {
            before.next = node;
            node.prev = before;
        }

    }

    pub fn insert_head(&mut self, node: &mut List) {
        if let Some(head) = self.node_mut(self.next) {
            head.prev = node;
            node.next = head;
            self.next = node;
        } else {
            panic!("Bad node address {:X}", node as *const _ as usize);
        }
    }

    pub fn insert_between(&mut self, before: &mut List, after: &mut List, node: &mut List) {
        before.next = node;
        after.prev = node;
        node.prev = before;
        node.next = after;
    }

    pub fn insert_tail(&mut self, node: &mut List) {
        if let Some(head) = self.node_mut(self.prev) {
            head.prev = node;
            node.next = head;
            self.next = node;
        } else {
            panic!("Bad node address {:X}", node as *const _ as usize);
        }
    }

    pub fn insert(&mut self, node: &mut List) {
        if let Some(after) = node.iter().find(|&n| {
            if n as usize > node as *const _ as usize {
                return true;
            }
            false
        }) {
            if let Some(after) = unsafe{ after.as_mut() } {
                if let Some(before) = unsafe {after.prev.as_mut()} {
                    before.next = node;
                    after.prev = node;
                    node.prev = before;
                    node.next = after;
                } else {
                    // No node before. Must be head
                    node.next = after;
                    node.prev = after.prev;
                    after.prev = node;
                    self.next = node;
                }
            }
        } else {
            if let Some(tail) = unsafe{self.prev.as_mut()} {
                // No node after. Must be tail
                self.prev = node;
                node.prev = tail;
                node.next = tail.next;
                tail.next = node;
            } else {
                // No node in the list;
                self.next = node;
                self.prev = node;
                node.next = null_mut();
                node.prev = null_mut();
            }
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
    next: *mut List,
}

impl Iterator for ListIter {
    type Item = *mut List;
    fn next(&mut self) -> Option<Self::Item> {
        if self.current == null_mut() {
            return None;
        }

        let ret = self.current;
        self.current = self.next;
        if let Some(next_node) = unsafe{self.next.as_mut()} {
            self.next = next_node.next;
        }

        Some(ret)
    }
}

impl DoubleEndedIterator for ListIter {
    fn next_back(&mut self) -> Option<Self::Item> {
        let Some(curr) = (unsafe{self.current.as_ref()}) else {
            return None;
        };
        let ret = self.current;
        self.current = curr.prev;
        Some(ret)
    }
}
