use common::address::VirtualAddress;

pub struct RawList {
    next: *mut RawList,
    prev: *mut RawList,
}

impl Iterator for &RawList {
    type Item = *mut RawList;
    fn next(&mut self) -> Option<Self::Item> {
        if self.next == core::ptr::null_mut() {
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

        //ret.next = &mut ret;
        //ret.prev = &mut ret;
        //ret
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

    pub fn init(x: &mut RawList) -> &mut RawList {
        x.next = x;
        x.prev = x;
        x
    }

    pub fn insert(&mut self, list: &mut RawList) {
        if self.is_empty() {
            self.next = list;
            self.prev = list;
        }

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
