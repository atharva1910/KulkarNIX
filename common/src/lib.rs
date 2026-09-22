#![no_std]
use r_efi::protocols::graphics_output::ModeInformation;
use crate::address::{PhysicalAddress, VirtualAddress};

pub mod paging;
pub mod hal;
pub mod serial_port;
pub mod address;

pub const KERNEL_ARGS_PAGES: usize = 10;
pub const MMAP_BUFFER_SIZE: usize = KERNEL_ARGS_PAGES * paging::PAGE_SIZE - (core::mem::size_of::<usize>() * 2);
pub const KERNEL_CS_ADDR: usize = 0xFFFF_FA00_0000_0000;
pub const KERNEL_DS_ADDR: usize = 0xFFFF_8000_0000_0000;

#[repr(C)]
pub struct FrameBuffer {
    pub frame_base: u64,
    pub frame_size: usize,
    pub mode_information: ModeInformation,
}

#[repr(C)]
pub struct KernelArgs {
    //    pub frame_buf_info: FrameBuffer,
    pub desc_size: usize,
    pub num_desc: usize,
    pub mem_map_size: usize,
    pub total_memory: usize,
    pub buffer: PhysicalAddress,
    pub kernel_pbase: PhysicalAddress,
    pub kernel_vbase: VirtualAddress,
    pub kernel_pages: usize,
}
