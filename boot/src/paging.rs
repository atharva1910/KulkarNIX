use core::sync::atomic::AtomicPtr;
use r_efi::efi;
use crate:: memory_map::MemoryMap;
pub const PAGE_TABLE_NUM_ENTRIES: usize = 512;

#[repr(transparent)]
pub struct PageEntry(pub u64);

#[repr(C, align(4096))]
pub struct PML4T {
    pub pml4e: [PageEntry; PAGE_TABLE_NUM_ENTRIES]
}

#[repr(C, align(4096))]
pub struct PDPT {
    pub pdpe: [PageEntry; PAGE_TABLE_NUM_ENTRIES]
}

#[repr(C, align(4096))]
pub struct PDT {
    pub pde: [PageEntry; PAGE_TABLE_NUM_ENTRIES]
}

#[repr(C, align(4096))]
pub struct PT {
    pub pte: [PageEntry; PAGE_TABLE_NUM_ENTRIES]
}


impl PageEntry {
    const PRESENT: u64 = 1 << 0;
    const RW: u64 = 1 << 1;
    const ADDR_MASK: u64 = (1 << 12) - 1;

    pub fn set_present(&mut self) {
        self.0 |= Self::PRESENT;
    }

    pub fn set_rw(&mut self) {
        self.0 |= Self::RW;
    }

    fn clear_addr(&mut self) {
    }

    pub fn set_addr(&mut self, addr: u64) {
        assert!(addr & Self::ADDR_MASK == 0, "ADDRESS NOT 4KB ALIGNED");
        self.0 |= addr;
    }
}

impl PDPT {
    const PS: u64 = 1 << 7;
    const ADDR_MASK: u64 = (1 << 30) - 1;
    const PRESENT: u64 = 1 << 0;
    const RW: u64 = 1 << 1;

    pub fn set_1gb_paging(&mut self, idx: usize, addr: u64) {
        assert!(addr & Self::ADDR_MASK == 0, "ADDRESS NOT 1GB ALIGNED");
        self.pdpe[idx].0 = Self::PS | Self::PRESENT | Self::RW | addr;
    }
}

impl PDT {
    const PS: u64 = 1 << 7;
    pub fn set_2mb_paging(&mut self, idx: usize, addr: u64) {
        self.pde[idx].0 |= Self::PS;
    }
}

pub struct PageTableManager {
    cr3_base: *mut PML4T
}

impl PageTableManager {
    pub fn new(page: u64) -> Self {
        let cr3_base = page as *mut PML4T;
        Self{
            cr3_base
        }
    }

    pub fn get_pml4t(&self) -> Option<&mut PML4T> {
        unsafe { self.cr3_base.as_mut() }
    }
}
