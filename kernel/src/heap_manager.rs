use core::mem::offset_of;
use crate::{linked_list::List, pmem_manager::PMemManager};
use common::{address:: VirtualAddress, paging::PAGE_SIZE};

#[repr(C)]
struct MetaData {
    size: usize,
    links: List,
}

impl MetaData {
    pub fn is_adjacent(&mut self, node2: &mut MetaData) -> bool {
        assert!((self as *const _ as usize) < node2 as *const _ as usize);
        let node2_start = node2 as *const _ as usize;
        let node1_list = &self.links as *const _ as usize;
        node1_list + self.size == node2_start
    }
}

pub struct HeapManager {
    free_list: *mut List,
}

impl HeapManager {
    pub fn init() -> Self {
        Self {
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
        let Some(node) = self.get_node_mut(addr) else {
            panic!("Bad node address {:X}", addr);
        };

        let Some(list) = self.get_list_mut() else {
            self.free_list = &mut node.links;
            return;
        };

        let mut before: Option<&mut MetaData> = None;
        let mut after: Option<&mut MetaData> = None;

        for link in list.iter() {
            let Some(itr_node) = self.get_node_mut(VirtualAddress::from(link as usize)) else {
                panic!("Bad node address {:X}", addr);
            };

            if (itr_node as *const _ as usize) < *addr {
                before = Some(itr_node);
                continue;
            } else {
                after = Some(itr_node);
                break;
            }
        }

        if let Some(node_after) = after {
            if node.is_adjacent(node_after) {
                node.size += node_after.size;
                list.remove(&mut node_after.links);
            }
        }

        if let Some(node_before) = before {
            if node_before.is_adjacent(node) {
                node_before.size += node.size;
            } else {
                list.insert_after(&mut node_before.links, &mut node.links);
            }
        }
    }
}

impl HeapManager {
    #[inline]
    pub fn get_list_mut(&self) -> Option<&mut List> {
        unsafe {
            self.free_list.as_mut()
        }
    }

    #[inline]
    fn get_node_ref(&self, addr: VirtualAddress) -> Option<&MetaData> {
        unsafe {
            self.get_node(addr).as_ref()
        }
    }

    #[inline]
    fn get_node_mut(&self, addr: VirtualAddress) -> Option<&mut MetaData> {
        unsafe {
            self.get_node(addr).as_mut()
        }
    }

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

        let Some(addr) = PMemManager::alloc_pages(num_pages) else {
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

    fn get_node(&self, addr: VirtualAddress) -> *mut MetaData {
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


}
