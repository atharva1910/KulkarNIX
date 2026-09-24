use common::{address::VirtualAddress, paging::PAGE_SIZE};
use crate::{linked_list::RawList, pmem_manager::PMemManager};

#[repr(C)]
struct MetaData {
    size: usize,
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
        let num_pages = usize::div_ceil(size + size_of::<usize>(), PAGE_SIZE);
        let Some(addr) = self.pmm.alloc_pages(num_pages) else {
            return None;
        };

        let true_size = num_pages << 12;
        let pmeta_data = addr.get_raw() as *mut MetaData;
        unsafe {
            (*pmeta_data).size = true_size;
        }

        Some(VirtualAddress(addr.get_raw() + size_of::<MetaData>()))
    }

    pub fn free(&mut self, addr: VirtualAddress) {
        let node = RawList::create_node(addr);
        self.free_list.insert(node);
    }
}
