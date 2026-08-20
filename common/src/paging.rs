pub const PAGE_TABLE_NUM_ENTRIES: usize = 512;
pub const PAGE_SIZE: usize = 4096;

#[repr(transparent)]
pub struct VAddr(u64);

impl VAddr{
    pub fn pt_idx(&self) -> usize{
        (self.0 as usize >> 12) & 0x1FF
    }
    pub fn pdt_idx(&self) -> usize{
        (self.0 as usize >> 21) & 0x1FF
    }
    pub fn pdpt_idx(&self) -> usize{
        (self.0 as usize >> 30) & 0x1FF
    }
    pub fn pml4_idx(&self) -> usize{
        (self.0 as usize >> 39) & 0x1FF
    }
}

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
    const ADDR_ALIGNMENT: u64 = (1 << 12) - 1;
    const ADDR_MASK:u64 = 0x000F_FFFF_FFFF_F000;

    pub fn set_present(&mut self) {
        self.0 |= Self::PRESENT;
    }

    pub fn set_rw(&mut self) {
        self.0 |= Self::RW;
    }

    fn clear_addr(&mut self) {
        self.0 &= Self::ADDR_MASK;
    }

    pub fn set_addr(&mut self, addr: u64) {
        assert!(addr & Self::ADDR_ALIGNMENT == 0, "ADDRESS NOT 4KB ALIGNED");
        self.0 &= !Self::ADDR_MASK;
        self.0 |= addr;
    }

    pub fn get_addr(&mut self) -> u64 {
        self.0 & Self::ADDR_MASK
    }

    pub fn is_entry_present(&self) -> bool {
        self.0 != 0
    }
}

impl PDPT {
    const PS: u64 = 1 << 7;
    const ADDR_ALIGNMENT: u64 = (1 << 30) - 1;
    const PRESENT: u64 = 1 << 0;
    const RW: u64 = 1 << 1;

    pub fn set_1gb_paging(&mut self, idx: usize, addr: u64) {
        assert!(addr & Self::ADDR_ALIGNMENT == 0, "ADDRESS NOT 1GB ALIGNED");
        self.pdpe[idx].0 = Self::PS | Self::PRESENT | Self::RW | addr;
    }
}

impl PDT {
    const PS: u64 = 1 << 7;
    const ADDR_ALIGNMENT: u64 = (1 << 20) - 1;
    const PRESENT: u64 = 1 << 0;
    const RW: u64 = 1 << 1;
    pub fn set_2mb_paging(&mut self, idx: usize, addr: u64) {
        assert!(addr & Self::ADDR_ALIGNMENT == 0, "ADDRESS NOT 2MB ALIGNED");
        self.pde[idx].0 = Self::PS | Self::PRESENT | Self::RW | addr;
    }
}

pub struct PageTableManager<T>
where
    T: Fn(usize) -> Option<u64> {
    pml4t: *mut PML4T,
    page_allocator: T,
}

impl<T> PageTableManager<T>
where
    T: Fn(usize) -> Option<u64> {
    pub fn new(page_allocator: T) -> Option<Self> {
        let Some(pml4t) = page_allocator(1) else {
            return None;
        };

        Some(Self{
            pml4t: (pml4t as *mut PML4T),
            page_allocator
        })
    }

    pub fn get_pml4t_mut(&self) -> Option<&mut PML4T> {
        unsafe {
            self.pml4t.as_mut()
        }
    }

    pub fn get_pml4t(&self) -> Option<&PML4T> {
        unsafe {
            self.pml4t.as_ref()
        }
    }

    pub fn allocate_tables(&self, num_tables: usize) -> Option<u64> {
        (self.page_allocator)(num_tables) // No method found??
    }

    pub fn get_or_create_entry<TABLE>(&self, entry: &mut PageEntry) -> Option<&'static mut TABLE> {
        const ADDR_MASK:u64 = 0x000F_FFFF_FFFF_F000;
        let mut table_addr = entry.get_addr() & ADDR_MASK;
        if table_addr == 0 {
            table_addr = (self.page_allocator)(1)?;
            entry.set_addr(table_addr);
            entry.set_present();
            entry.set_rw();
        }
        unsafe {
            (table_addr as *mut TABLE).as_mut()
        }
    }

    pub fn map_page(&self, phy: u64, virt: u64) -> bool {
        let v = VAddr(virt);
        let Some(pml4t) = self.get_pml4t_mut() else {
            return false;
        };

        let Some(pdpt) = self.get_or_create_entry::<PDPT>(&mut pml4t.pml4e[v.pml4_idx()]) else {
            return false;
        };

        let Some(pdt) = self.get_or_create_entry::<PDT>(&mut pdpt.pdpe[v.pdpt_idx()]) else {
            return false;
        };

        let Some(pt) = self.get_or_create_entry::<PT>(&mut pdt.pde[v.pdt_idx()]) else {
            return false;
        };

        pt.pte[v.pt_idx()].set_rw();
        pt.pte[v.pt_idx()].set_present();
        pt.pte[v.pt_idx()].set_addr(phy);
        true
    }
}
