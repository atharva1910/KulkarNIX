use common::{address::VirtualAddress, paging::PAGE_SIZE};
use crate::pmem_manager::PMemManager;

#[repr(C)]
struct MetaData {
    size: usize,
    next: *mut MetaData,
    prev: *mut MetaData,
}

pub struct HeapManager<'a> {
    pmm: &'a mut PMemManager,
    heap_size: usize,
    free_size: usize,
    alloc_size: usize,
    free_list: MetaData,
}

impl<'a> HeapManager<'a> {
    pub fn init(pmm: &'a mut PMemManager) -> Self {
        Self {
            pmm,
            heap_size: 0,
            free_size: 0,
            alloc_size: 0,
            free_list: MetaData{
                size: 0,
                next: core::ptr::null_mut(),
                prev: core::ptr::null_mut(),
            }
        }
    }

    pub fn alloc(&mut self, size: usize) -> Option<VirtualAddress> {
        if size <= self.free_size {
            // Allocate internally
        }

        let num_pages = usize::div_ceil(size + size_of::<usize>(), PAGE_SIZE);
        if let Some(addr) = self.pmm.alloc_pages(num_pages) {
            self.heap_size += num_pages << 12;
            self.alloc_size += num_pages << 12;
            let ret = addr + size_of::<usize>();
            let p = addr.get_raw() as *mut MetaData;
            unsafe {
                (*p).size = size;
                (*p).next = core::ptr::null_mut();
                (*p).prev = core::ptr::null_mut();
            }
            return Some(ret);
        }

        None
    }

    pub fn free(&mut self, addr: VirtualAddress) {
        let pmeta_data = addr - size_of::<usize>();
        let p = pmeta_data.get_raw() as *mut MetaData;
        unsafe {
            // Todo add to free list
            self.free_list.size += (*p).size;
            (*p).next = self.free_list.next;
            (*p).prev = self.free_list.prev;
        }
    }
}
