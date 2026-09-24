use common::address::VirtualAddress;

pub struct RawList {
    next: *mut RawList,
    prev: *mut RawList,
}

impl RawList {
    pub fn new() -> Self {
        Self {
            next: core::ptr::null_mut(),
            prev: core::ptr::null_mut(),
        }
    }

    pub fn is_empty(&self) -> bool {
        self.next == self.prev
    }

    pub fn remove(&mut self) {
        let next_node = self.next;
        let prev_node = self.prev;
        unsafe {
            (*prev_node).next = next_node;
            (*next_node).prev = prev_node;
        }
    }

    pub fn insert(&mut self, list: *mut RawList) {
        unsafe {
            let tail = (*list).prev;
            (*tail).next = self;
            (*list).prev = self;
            self.prev = tail;
            self.next = list;
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
