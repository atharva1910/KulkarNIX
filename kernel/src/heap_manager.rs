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
        let Some(node) = self.get_node_mut(addr) else {
            panic!("Bad node address {:X}", addr);
        };

        let Some(list) = self.get_list_mut() else {
            self.free_list = &mut node.links;
            return;
        };

        for link in list.iter() {
            let Some(itr_node) = self.get_node_mut(VirtualAddress::from(link as usize)) else {
                panic!("Bad node address {:X}", addr);
            };

            if (itr_node as *const _ as usize) < *addr {
                let next_node = itr_node.links.get_next();
                if let Some(next_node) = self.get_node_mut(VirtualAddress::from(next_node as usize)) {
                    list.insert_between(&mut itr_node.links, &mut next_node.links, &mut node.links);
                } else {
                    list.insert_tail(&mut node.links);
                }
            } else {
                if node.size + *addr == link as usize {
                } else {
                    list.insert_head(&mut node.links);
                }
            }
        }
    }
}

impl<'a> HeapManager<'a> {
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
