use crate::{bitmap::BitMap, errors::KError};
use common::{
    KernelArgs,
    address::{PhysicalAddress, VirtualAddress},
    paging::PAGE_SIZE,
};
use r_efi::efi::{
    BOOT_SERVICES_DATA, CONVENTIONAL_MEMORY, LOADER_CODE, LOADER_DATA, MemoryDescriptor,
};

pub struct PMemManager {
    bitmap: &'static mut [u8],
    num_bits: usize,
}

impl PMemManager {
    fn is_page_free(&self, pos: usize) -> bool {
        let byte_pos = pos >> 3;
        let bit_pos = byte_pos & !0x8;
        self.bitmap[byte_pos] & (1 << bit_pos) == 0
    }

    fn mark_page_alloc(&mut self, pos: usize) {
        let byte_pos = pos >> 3;
        let bit_pos = byte_pos & !0x8;
        self.bitmap[byte_pos] |= 1 << bit_pos;
    }

    pub fn alloc_page(&mut self, n: usize) -> Option<VirtualAddress> {
        let mut itr: usize = 0;
        let mut found: usize = 0;

        loop {
            if itr > self.bitmap.len() {
                break;
            }

            if !self.is_page_free(itr) {
                itr = itr + 1;
                found = 0;
                continue;
            }

            found = found + 1;
            itr = itr + 1;

            if found == n {
                break;
            }
        };

        if found != n {
            return None;
        }

        let start = itr - found;
        (0..n).for_each(|i|
                        self.mark_page_alloc(start + i));

        Some(PhysicalAddress(start << 12).to_virtual())
    }

    fn free_page(&mut self, addr: VirtualAddress) {
        assert!(addr.get_raw() % PAGE_SIZE == 0);
        let bit_pos = addr.get_raw() >> 12;
        let byte_pos = bit_pos >> 3;
        let bit_pos = bit_pos & 0x7;
        self.bitmap[byte_pos] &= !(1 << bit_pos);
    }

    pub fn free_pages(&mut self, addr: VirtualAddress, n: usize) {
        assert!(addr.get_raw() % PAGE_SIZE == 0);
        (0..n).for_each(|i| self.free_page(addr + (i * PAGE_SIZE)));
    }

    pub fn init(kernel_args: *const KernelArgs) -> Result<Self, KError> {
        let Some(pargs) = (unsafe { kernel_args.as_ref() }) else {
            return Err(KError::GeneralFaliure);
        };

        let total_pages = usize::div_euclid(pargs.total_memory, PAGE_SIZE);
        let bytes_required = usize::div_ceil(total_pages, 8);
        let pages_required = usize::div_ceil(bytes_required, PAGE_SIZE);

        let num_desc = pargs.mem_map_size / pargs.desc_size;

        let Some(bitmap_paddr) = (0..num_desc).find_map(|i| {
            let desc_paddr = pargs.buffer + (i * pargs.desc_size);
            let desc_vaddr = VirtualAddress::from(desc_paddr);
            let pdesc = desc_vaddr.get_raw() as *const MemoryDescriptor;

            let Some(desc) = (unsafe { pdesc.as_ref() }) else {
                return None;
            };

            if desc.r#type != CONVENTIONAL_MEMORY
                && desc.r#type != BOOT_SERVICES_DATA
                && desc.r#type != LOADER_CODE
                && desc.r#type != LOADER_DATA
                && (desc.number_of_pages as usize) < pages_required
            {
                return None;
            }

            Some(PhysicalAddress(desc.physical_start as usize))
        }) else {
            return Err(KError::GeneralFaliure);
        };

        let mut pmm = unsafe {
            Self {
                bitmap: core::slice::from_raw_parts_mut(
                    VirtualAddress::from(bitmap_paddr).get_raw() as *mut u8,
                    bytes_required,
                ),
                num_bits: total_pages
            }
        };

        pmm.bitmap.fill(u8::MAX);

        (0..num_desc).for_each(|i| {
            let desc_paddr = pargs.buffer + (i * pargs.desc_size);
            let desc_vaddr = VirtualAddress::from(desc_paddr);
            let pdesc = desc_vaddr.get_raw() as *const MemoryDescriptor;
            let Some(desc) = (unsafe { pdesc.as_ref() }) else {
                return;
            };

            if desc.r#type != CONVENTIONAL_MEMORY
                && desc.r#type != BOOT_SERVICES_DATA
                && desc.r#type != LOADER_CODE
                && desc.r#type != LOADER_DATA
            {
                return;
            }

            pmm.free_pages(
                PhysicalAddress(desc.physical_start as usize).to_virtual(),
                desc.number_of_pages as usize,
            );
        });


        Ok(pmm)
    }
}
