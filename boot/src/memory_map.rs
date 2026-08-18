use r_efi::efi::{self, LOADER_DATA, BOOT_SERVICES_CODE,BOOT_SERVICES_DATA, CONVENTIONAL_MEMORY, LOADER_CODE, MemoryDescriptor};
use crate::{
    boot_ctx::BOOT_CTX,
    printer::PRINTER
};
use alloc::{format, vec};
pub struct MemoryMap {
    pub total_memory: usize,
    pub min_vaddr: u64,
    pub min_paddr: u64,
}

impl MemoryMap {
    pub fn new() -> Result<Self, efi::Status> {
        let Some(bs) = BOOT_CTX.get_bs() else {
            PRINTER.print("GET BS FAIL\n");
            return Err(efi::Status::INVALID_PARAMETER);
        };

        let mut desc_size: usize = 0;
        let mut mem_map_size: usize = 0;
        let mut key: usize = 0;
        let mut desc_version: u32 = 0;
        unsafe {
            (bs.get_memory_map)(&mut mem_map_size,
                                core::ptr::null_mut(),
                                &mut key,
                                &mut desc_size,
                                &mut desc_version);
        }

        mem_map_size += 4096;
        let mut mem_map = vec![0 as u8; mem_map_size];
        let status = unsafe {
            (bs.get_memory_map)(&mut mem_map_size,
                                mem_map.as_mut_ptr().cast(),
                                &mut key,
                                &mut desc_size,
                                &mut desc_version)
        };
        if status != efi::Status::SUCCESS {
            PRINTER.print(&format!("status: {} memory_map size : {} desc_size {} desc_version {} \n", status, mem_map_size, desc_size, desc_version));
            return Err(status);
        }

        let mut total_memory: usize = 0;
        let mut min_paddr = 0;
        let mut min_vaddr = 0;
        for chunk in mem_map[..mem_map_size].chunks_exact(desc_size) {
            let desc = unsafe {
                chunk.as_ptr().cast::<MemoryDescriptor>().as_ref().unwrap()
            };

            if  desc.r#type != CONVENTIONAL_MEMORY &&
                desc.r#type != BOOT_SERVICES_DATA &&
                desc.r#type != LOADER_CODE &&
                desc.r#type != LOADER_DATA {
                    continue;
                }

            total_memory +=
                (desc.number_of_pages << 12) as usize;

            if desc.physical_start < min_paddr {
                min_paddr = desc.physical_start;
            }

            if desc.virtual_start < min_vaddr {
                min_vaddr = desc.virtual_start;
            }

        }

        Ok(Self{
            total_memory,
            min_vaddr,
            min_paddr,
        })
    }
}
