use common::{KernelArgs, address::VirtualAddress};
use r_efi::efi::{self, LOADER_DATA, BOOT_SERVICES_DATA, CONVENTIONAL_MEMORY, LOADER_CODE, MemoryDescriptor};
use crate::serial_port;

pub struct PMemManager {
}

static PMEM_MANAGER: PMemManager = PMemManager{};

impl PMemManager {
    pub fn init(kernel_args: *const KernelArgs) -> bool {
        let Some(pargs) = (unsafe {kernel_args.as_ref()}) else {
            return false;
        };

        let total_pages = pargs.total_memory >> 12;
        let bytes_required = total_pages >> 3;
        let pages_required = bytes_required >> 12;
        let num_desc = pargs.mem_map_size/pargs.desc_size;

        for i in 0..num_desc {

            let desc_paddr = pargs.buffer + (i * pargs.desc_size);
            let desc_vaddr = VirtualAddress::from(desc_paddr);
            let pdesc = desc_vaddr.get_raw() as *const MemoryDescriptor;

            let Some(desc) = (unsafe{pdesc.as_ref()}) else {
                continue;
            };

            if  desc.r#type != CONVENTIONAL_MEMORY &&
                desc.r#type != BOOT_SERVICES_DATA &&
                desc.r#type != LOADER_CODE &&
                desc.r#type != LOADER_DATA {
                    continue;
                }

            if (desc.number_of_pages as usize) < pages_required {
                continue;
            }

            serial_port::write("Selecting desc\n");
            break;
        }
        true
    }

    pub fn get() -> &'static Self {
        &PMEM_MANAGER
    }
}
