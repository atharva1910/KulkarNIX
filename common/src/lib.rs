#![no_std]
pub mod paging;
use r_efi::efi::MemoryDescriptor;
extern crate alloc;
use alloc::vec;

const MMAP_BUFFER_SIZE: usize = paging::PAGE_SIZE - (core::mem::size_of::<usize>() * 2);

#[repr(C)]
pub struct KernelArgs {
    pub desc_size: usize,
    pub mem_map_size: usize,
    pub buffer: [u8; MMAP_BUFFER_SIZE],
}
