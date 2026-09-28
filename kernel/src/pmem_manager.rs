use core::{fmt::Write, usize};
use crate::{errors::KError, SPrint, spin_lock::SpinLock};
use common::{
    KERNEL_ARGS_PAGES, KernelArgs, address::{PhysicalAddress, VirtualAddress}, paging::PAGE_SIZE
};
use r_efi::efi::{
    BOOT_SERVICES_DATA, CONVENTIONAL_MEMORY, LOADER_CODE, LOADER_DATA, MemoryDescriptor,
};

pub struct PMemManager {
    bitmap: &'static mut [u8],
    num_bits: usize,
}

static PMEM_MGR: SpinLock<Option<PMemManager>> = SpinLock::init(None);

impl PMemManager {
    pub fn free_pages(addr: VirtualAddress, n: usize) {
        assert!(addr.get_raw() % PAGE_SIZE == 0, "addr not page_size");
        if let Some(pmm) = PMEM_MGR.lock().as_mut() {
            (0..n).for_each(|i| pmm.free_page(addr + (i * PAGE_SIZE)));
        }
    }

    pub fn alloc_pages(n: usize) -> Option<VirtualAddress> {
        assert!(n != 0);
        let mut guard = PMEM_MGR.lock();
        let Some(pmm) =  guard.as_mut() else {
            return None;
        };
        let mut itr: usize = 0;
        let mut found: usize = 0;

        loop {
            if itr >= pmm.num_bits {
                break;
            }

            if !pmm.is_page_free(itr) {
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
        SPrint!("Found {} pages at bit {}. Addr 0x{:x}", n, start, start << 12);
        (0..n).for_each(|i|
                        pmm.mark_page_alloc(start + i));

        Some(PhysicalAddress::from(start << 12).to_virtual())
    }

    fn is_page_free(&self, pos: usize) -> bool {
        let byte_pos = pos >> 3;
        let bit_pos = pos & 0x7;
        self.bitmap[byte_pos] & (1 << bit_pos) == 0
    }

    fn mark_page_alloc(&mut self, pos: usize) {
        let byte_pos = pos >> 3;
        let bit_pos = pos & 0x7;
        self.bitmap[byte_pos] |= 1 << bit_pos;
    }

    fn mark_page_free(&mut self, pos: usize) {
        let byte_pos = pos >> 3;
        let bit_pos = pos & 0x7;
        self.bitmap[byte_pos] &= !(1 << bit_pos);
    }

    fn free_page(&mut self, addr: VirtualAddress) {
        let bit_pos = addr.to_physical().get_raw() >> 12;
        let byte_pos = bit_pos >> 3;
        let bit_pos = bit_pos & 0x7;
        self.bitmap[byte_pos] &= !(1 << bit_pos);
    }

    pub fn init(args_addr: VirtualAddress) -> Result<(), KError> {
        let kernel_args = args_addr.get_raw() as *const KernelArgs;
        let Some(pargs) = (unsafe { kernel_args.as_ref() }) else {
            return Err(KError::GeneralError);
        };


        let total_pages = usize::div_euclid(pargs.total_memory, PAGE_SIZE);
        let bytes_required = usize::div_ceil(total_pages, 8);
        let pages_required = usize::div_ceil(bytes_required, PAGE_SIZE);
        SPrint!("total_pages {:X} bytes_required {:X} pages_required {:X}", total_pages, bytes_required, pages_required);

        let num_desc = usize::div_euclid(pargs.mem_map_size, pargs.desc_size);

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
            {
                return None;
            }

            if (desc.number_of_pages as usize) < pages_required {
                return None;
            }

            Some(PhysicalAddress::from(desc.physical_start as usize))
        }) else {
            return Err(KError::GeneralError);
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

        // Free usable memory
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

            let page_pos = desc.physical_start as usize >> 12;
            (0..desc.number_of_pages as usize).for_each(|i| pmm.mark_page_free(page_pos + i));
        });

        // Mark kernel address allocated
        assert!(*pargs.kernel_pbase % PAGE_SIZE == 0);
        SPrint!("Marking Kernel Memory as allocated {:X}", *pargs.kernel_pbase);
        let kernel_base_pos = *pargs.kernel_pbase >> 12;
        (0..pargs.kernel_pages).for_each(|i| pmm.mark_page_alloc(kernel_base_pos + i));

        // Mark the kernel arguments pages as allocated
        assert!(*args_addr.to_physical() as usize % PAGE_SIZE == 0);
        (0..KERNEL_ARGS_PAGES).for_each(|i| pmm.mark_page_alloc((*args_addr.to_physical() >> 12) + i));

        // Mark the bitmap memory as used
        assert!(*bitmap_paddr % PAGE_SIZE == 0);
        (0..pages_required).for_each(|i| pmm.mark_page_alloc((bitmap_paddr.get_raw() >> 12) + i));

        *PMEM_MGR.lock() = Some(pmm);

        Ok(())
    }
}
