use core::mem::offset_of;
use core::fmt::Write;
use crate::{SPrint, linked_list::List, pmem_manager::PMemManager};
use common::{address::{PhysicalAddress, VirtualAddress}, paging::PAGE_SIZE};

#[repr(C)]
struct MetaData {
    size: usize,
    links: List,
}

pub struct HeapManager<'a> {
    pmm: &'a mut PMemManager,
    free_list: *mut List,
}

impl<'a> HeapManager<'a> {
    pub fn init(pmm: &'a mut PMemManager) -> Self {
        Self {
            pmm,
            free_list: core::ptr::null_mut(),
        }
    }

    pub fn alloc(&mut self, size: usize) -> Option<VirtualAddress> {
        if let Some(addr) = self.check_free_list(size) {
            return Some(addr);
        }

        if !self.add_mem(size) {
            return None;
        }

        SPrint!("Checkig free list again");
        if let Some(addr) = self.check_free_list(size) {
            return Some(addr);
        }

        None
    }

    pub fn free(&mut self, addr: VirtualAddress) {
        //if let Some(node) = List::create_node(addr).as_ref() {
        //    if let Some(list) = unsafe{self.free_list.as_mut()} {
        //        list.insert(&mut node.links);
        //    }
        //}
    }
}

impl<'a> HeapManager<'a> {
    fn get_vaddress(&self, meta_data: &mut MetaData) -> Option<VirtualAddress> {
        let ret = &meta_data.links as *const _ as usize;
        return Some(VirtualAddress(ret));
    }

    fn num_pages(&self, size: usize) -> usize {
        let num_pages = usize::div_ceil(size + size_of::<usize>(), PAGE_SIZE);
        num_pages
    }

    fn add_mem(&mut self, size: usize) -> bool {
        let num_pages = self.num_pages(size);
        SPrint!("Allocating num pages: {}", num_pages);

        let Some(addr) = self.pmm.alloc_pages(num_pages) else {
            return false;
        };

        SPrint!("Allocated {} page at addr: {:X}", num_pages, addr);
        let alloc_size = num_pages << 12 - size_of::<usize>();
        self.create_node(addr, alloc_size);
        true
    }

    fn check_free_list(&mut self, size: usize) -> Option<VirtualAddress> {
        let Some(list) = (unsafe{self.free_list.as_ref()}) else {
            return None;
        };

        if let Some(pmd) = list.iter()
            .filter_map(|node| {
                let base = node as usize - offset_of!(MetaData, links);
                if let Some(pmd) = unsafe {(base as *mut MetaData).as_mut()} {
                    return Some(pmd);
                }
                return None;
            })
            .find(|pmd| pmd.size >= size) {
                self.chop_node(pmd, size);
                return self.get_vaddress(pmd);
            }


        SPrint!("No memory found in free list");
        None
    }

    fn chop_node(&mut self, node: &mut MetaData, size: usize)  {
        let rem = node.size - size;
        SPrint!("Chopping Node. Node Size {} Req Size {} Rem {}", node.size, size, rem);
        if rem <= size_of::<MetaData>() {
            return;
        }

        // Shirk the current node
        node.size = size;

        // Create new node
        let addr = VirtualAddress(node as *const _ as usize + size);
        self.create_node(addr, rem);
    }

    fn create_node(&mut self, addr: VirtualAddress, size: usize) {
        SPrint!("Creating Node. Addr {:X} size {}", addr, size);
        let pmeta_data = *addr as *mut MetaData;
        if let Some(pmd) = unsafe {pmeta_data.as_mut()} {
            pmd.size = size;
            if let Some(list) = unsafe{self.free_list.as_mut()} {
                list.insert(&mut pmd.links);
            }
        }
    }
}
