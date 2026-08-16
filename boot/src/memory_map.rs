use r_efi::efi::{self, MemoryDescriptor};
use crate::boot_ctx::BOOT_CTX;
use alloc::vec;
pub struct MemoryMap {
    pub total_memory: usize,
}

impl MemoryMap {
    pub fn new() -> Result<Self, efi::Status> {
        let Some(bs) = BOOT_CTX.get_bs() else {
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

        let mut mem_map = vec![0 as u8; mem_map_size];
        let status = unsafe {
            (bs.get_memory_map)(&mut mem_map_size,
                                mem_map.as_mut_ptr().cast(),
                                &mut key,
                                &mut desc_size,
                                &mut desc_version)
        };
        if status != efi::Status::SUCCESS {
            return Err(status);
        }

        let mut total_memory: usize = 0;
        for chunk in mem_map[..mem_map_size].chunks_exact(desc_size) {
            let desc = chunk.as_ptr().cast::<MemoryDescriptor>();
            total_memory += unsafe {
                ((*desc).number_of_pages << 12) as usize
            }
        }

        Ok(Self{
            total_memory
        })
    }
}
