#![no_std]
pub mod paging;
extern crate alloc;
use alloc::vec;

pub struct KernelArgs {
    pub key: usize,
    pub buffer: vec::Vec<u8>,
}
