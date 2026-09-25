use core::mem::offset_of;
use core::fmt::Write;
use crate::{SPrint, linked_list::RawList, pmem_manager::PMemManager};
use common::{address::VirtualAddress, paging::PAGE_SIZE};

#[repr(C)]
struct MetaData {
    size: usize,
    links: RawList,
}

pub struct HeapManager<'a> {
    pmm: &'a mut PMemManager,
    free_list: RawList,
}

impl<'a> HeapManager<'a> {
    pub fn init(pmm: &'a mut PMemManager) -> Self {
        Self {
            pmm,
            free_list: RawList::new(),
        }
    }

    pub fn alloc(&mut self, size: usize) -> Option<VirtualAddress> {
        if let Some(addr) = self.check_free_list(size) {
            return Some(addr.into());
        }

        let num_pages = usize::div_ceil(size + size_of::<usize>(), PAGE_SIZE);
        if !self.add_pages(num_pages) {
            // Failed to add pages
            return None;
        }

        if let Some(addr) = self.check_free_list(size) {
            return Some(addr.into());
        }

        None
    }

    pub fn free(&mut self, addr: VirtualAddress) {
        let node = RawList::create_node(addr);
        self.free_list.insert(node);
    }
}

impl<'a> HeapManager<'a> {
    fn add_pages(&mut self, num_pages: usize) -> bool {
        SPrint!("Allocating num pages: {}", num_pages);
        let Some(addr) = self.pmm.alloc_pages(num_pages) else {
            return false;
        };

        let pmeta_data = *addr as *mut MetaData;
        let size = num_pages << 12 - size_of::<usize>();
        unsafe {
            (*pmeta_data).size = size;
            let mut links = RawList::init(&mut (*pmeta_data).links);
            self.free_list.insert(&mut links);
        }
        true
    }

    fn check_free_list(&mut self, size: usize) -> Option<*mut MetaData> {
        for node in &self.free_list {
            let pmd = node as usize - offset_of!(MetaData, links);
            let pmd = pmd as *mut MetaData;
            unsafe {
                if (*pmd).size >= size {
                    return Some(self.chop_node(pmd, size));
                }
            }
        }

        SPrint!("No memory found in free list");
        None
    }

    fn chop_node(&mut self, node: *mut MetaData, size: usize) -> *mut MetaData {
        let Some( node_ref) = (unsafe { node.as_mut() }) else {
            return node; // This is a bug tho
        };

        let rem = node_ref.size - size;
        if rem <= size_of::<MetaData>() {
            // Way too small memory. Dont chop
            return node;
        }

        // chop chop
        node_ref.size = size; // Is this calc correct
        let new_node = (node as usize + size) as *mut MetaData;
        unsafe {
            (*new_node).size = rem;
            self.free_list.insert(&mut (*new_node).links);
        }

        return node;
    }
}
