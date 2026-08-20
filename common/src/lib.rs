#![no_std]

use r_efi::protocols::graphics_output::ModeInformation;
pub mod paging;
mod address;

pub const MMAP_BUFFER_SIZE: usize = paging::PAGE_SIZE - (core::mem::size_of::<usize>() * 2);

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
