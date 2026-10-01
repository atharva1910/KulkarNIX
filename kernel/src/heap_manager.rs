use core::{mem::offset_of, ptr::null_mut, fmt::Write};
use crate::{linked_list::List, pmem_manager::PMemManager, spin_lock::SpinLock, SPrint};
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

    pub fn as_addr(&self) -> usize {
        self as *const _ as usize
    }

    pub fn link_as_addr(&self) -> usize {
        &self.links as *const _ as usize
    }
}

pub struct HeapManager {
    free_list: *mut List,
}

unsafe impl Send for HeapManager{}

static HEAP_MGR: SpinLock<HeapManager> = SpinLock::init(HeapManager{
    free_list: null_mut()
});

impl HeapManager {
    pub fn alloc(size: usize) -> Option<VirtualAddress> {
        let mut hmm  = HEAP_MGR.lock();
        if let Some(addr) = hmm.check_free_list(size) {
            return Some(addr);
        }

        if !hmm.add_mem(size) {
            return None;
        }

        if let Some(addr) = hmm.check_free_list(size) {
            return Some(addr);
        }

        None
    }

    pub fn free(addr: VirtualAddress) {
        let mut hmm  = HEAP_MGR.lock();
        let Some(node) = hmm.get_node_from_links_mut(addr) else {
            panic!("Bad node address {:X}", addr);
        };

        let Some(list) = hmm.get_list_mut() else {
            hmm.free_list = &mut node.links;
            return;
        };

        let mut before: Option<&mut MetaData> = None;
        let mut after: Option<&mut MetaData> = None;

        for link in list.iter() {
            let Some(itr_node) = hmm.get_node_from_links_mut(VirtualAddress::from(link as usize)) else {
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

    pub fn print() {
        let  hmm  = HEAP_MGR.lock();
        let Some(list) = hmm.get_list_mut() else {
            return;
        };

        for link in list.iter() {
            if let Some(node) = hmm.get_node_from_links_mut(VirtualAddress::from(link as usize)) {
                SPrint!("Node[{:X}] Size: {} Links {:X}", node.as_addr(), node.size, node.link_as_addr());
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
    fn get_node_from_links_mut(&self, addr: VirtualAddress) -> Option<&mut MetaData> {
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
        let Some(free_list) = (unsafe{self.free_list.as_mut()}) else {
            return None;
        };

        let mut found_node_addr: Option<VirtualAddress> = None;
        for list in free_list.iter() {
            if let Some(node) = self.get_node_from_links_mut(VirtualAddress::from(list as usize)) {
                if node.size > size {
                    found_node_addr = Some(node.as_addr().into());
                    free_list.remove(&mut node.links);
                }
            } else {
                panic!("Bad Node address");
            }
        }

        if let Some(found_node_addr) = found_node_addr {
            self.chop_node(found_node_addr, size);
            if let Some(ret_node) = self.to_node(found_node_addr) {
                return self.get_vaddress(ret_node);
            }
        }

        None
    }

    fn chop_node(&mut self, addr: VirtualAddress, size: usize)  {
        let Some(node) = self.to_node(addr) else {
            panic!("Bad node address");
        };

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

    fn to_node(&self, addr: VirtualAddress) -> Option<&mut MetaData> {
        let node = *addr as *mut MetaData;
        unsafe {
            node.as_mut()
        }
    }
}
