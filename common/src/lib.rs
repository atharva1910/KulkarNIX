#![no_std]
use r_efi::protocols::graphics_output::ModeInformation;

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
    pub desc_size: usize,
    pub mem_map_size: usize,
    pub frame_buf_info: FrameBuffer,
    pub buffer: [u8; MMAP_BUFFER_SIZE],
}
