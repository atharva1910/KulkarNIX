use common::{KernelArgs, address::{PhysicalAddress, VirtualAddress}};
use r_efi::efi::{self, LOADER_DATA, BOOT_SERVICES_DATA, CONVENTIONAL_MEMORY, LOADER_CODE, MemoryDescriptor};
use crate::serial_port;

pub struct PMemManager {
    bitmap: Option<&'static mut [u8]>
}

static mut PMEM_MANAGER: PMemManager = PMemManager{
    bitmap: None
};

impl PMemManager {
    pub fn init(kernel_args: *const KernelArgs) -> bool {
        let Some(pargs) = (unsafe {kernel_args.as_ref()}) else {
            return false;
        };

        let total_pages = pargs.total_memory >> 12;
        let bytes_required = total_pages >> 3;
        let pages_required = bytes_required >> 12;

        let num_desc = pargs.mem_map_size/pargs.desc_size;
        let Some(bitmap_paddr) = (0..num_desc).find_map(|i| {

            let desc_paddr = pargs.buffer + (i * pargs.desc_size);
            let desc_vaddr = VirtualAddress::from(desc_paddr);
            let pdesc = desc_vaddr.get_raw() as *const MemoryDescriptor;

            let Some(desc) = (unsafe{pdesc.as_ref()}) else {
                return None;
            };

            if  desc.r#type != CONVENTIONAL_MEMORY &&
                desc.r#type != BOOT_SERVICES_DATA &&
                desc.r#type != LOADER_CODE &&
                desc.r#type != LOADER_DATA &&
                (desc.number_of_pages as usize) < pages_required
            {
                return None;
            }

            Some(desc.physical_start as usize)
        }) else {
            return false;
        };

        unsafe {
            PMEM_MANAGER.bitmap = Some(core::slice::from_raw_parts_mut(bitmap_paddr as *mut u8, bytes_required));
            if let Some(x) = PMEM_MANAGER.bitmap.as_deref_mut() {
                x.fill(0)
            }
        };
        true
    }
}
