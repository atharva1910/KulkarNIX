use core::mem::offset_of;
use crate::{linked_list::List, pmem_manager::PMemManager};
use common::{address:: VirtualAddress, paging::PAGE_SIZE};

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

        if let Some(addr) = self.check_free_list(size) {
            return Some(addr);
        }

        None
    }

    pub fn free(&mut self, addr: VirtualAddress) {
        if let Some(node) = unsafe{self.get_node(addr).as_mut()} {
            if let Some(list) = unsafe{self.free_list.as_mut()} {
                list.insert(&mut node.links);
            } else {
                self.free_list = &mut node.links;
            };
        } else {
            panic!("The node address is invalid?");
        };
    }
}

impl<'a> HeapManager<'a> {
    fn get_vaddress(&self, meta_data: &mut MetaData) -> Option<VirtualAddress> {
        let ret = &meta_data.links as *const _ as usize;
        return Some(VirtualAddress::from(ret));
    }

    fn num_pages(&self, size: usize) -> usize {
        let num_pages = usize::div_ceil(size + size_of::<usize>(), PAGE_SIZE);
        num_pages
    }

    fn add_mem(&mut self, size: usize) -> bool {
        let num_pages = self.num_pages(size);

        let Some(addr) = self.pmm.alloc_pages(num_pages) else {
            return false;
        };

        let alloc_size = (num_pages << 12) - size_of::<usize>();
        self.create_node(addr, alloc_size);
        true
    }

    fn check_free_list(&mut self, size: usize) -> Option<VirtualAddress> {
        let Some(list) = (unsafe{self.free_list.as_mut()}) else {
            return None;
        };

        if let Some(md) = list.iter()
            .filter_map(|node| {
                if let Some(pmd) = unsafe {(self.get_node(VirtualAddress::from(node as usize))).as_mut()} {
                    return Some(pmd);
                }
                return None;
            })
            .find(|pmd| {
                pmd.size >= size
            }) {
                self.chop_node(md, size);
                return self.get_vaddress(md);
            }


        None
    }

    fn chop_node(&mut self, node: &mut MetaData, size: usize)  {
        let rem = node.size - size;
        if rem <= size_of::<MetaData>() {
            return;
        }

        // Shirk the current node
        node.size = size;

        // Create new node
        let addr = VirtualAddress::from(node as *const _ as usize + size);
        self.create_node(addr, rem);
    }

    fn get_node(&mut self, addr: VirtualAddress) -> *mut MetaData {
        let base = *addr - offset_of!(MetaData, links);
        base as *mut MetaData
    }

    fn create_node(&mut self, addr: VirtualAddress, size: usize) {
        let pmeta_data = *addr as *mut MetaData;
        if let Some(pmd) = unsafe {pmeta_data.as_mut()} {
            pmd.size = size;
            if let Some(list) = unsafe{self.free_list.as_mut()} {
                list.insert(&mut pmd.links);
            } else {
                self.free_list = &mut pmd.links;
            }
        }
    }

    fn coallase_mem(&mut self) {

        let Some(list) = (unsafe{self.free_list.as_mut()}) else {
            panic!("List is empty?");
        };

        list.iter().fold(None, |prev, curr| {
            if prev.is_none() {
                return Some(curr);
            }

            if let Some(prev_node) = unsafe{self.get_node(VirtualAddress::from(prev.unwrap() as *const _ as usize)).as_mut()} {
                if let Some(curr_node) = unsafe{self.get_node(VirtualAddress::from(curr as usize)).as_mut()} {
                    if prev_node.size + prev_node as *const _ as usize == curr_node as *const _ as usize {
                        prev_node.size += curr_node.size;
                        list.remove(&mut curr_node.links);
                        return Some(&mut prev_node.links);
                    } else {
                        return Some(&mut curr_node.links);
                    }
                }
            }
            None
        });
    }
}
