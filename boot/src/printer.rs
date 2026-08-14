use r_efi::efi::{self, protocols::simple_text_output};
use crate::{
    boot_ctx::BOOT_CTX,
    elfheader::{ELF_MAGIC, Elf64Ehdr},
};
use core::sync::atomic::{AtomicPtr, Ordering};
use alloc::vec::Vec;

pub struct Printer {
    cout: AtomicPtr<simple_text_output::Protocol>
}

pub static PRINTER: Printer = Printer{
    cout: AtomicPtr::new(core::ptr::null_mut()),
};

impl Printer {
    pub fn init(&self, st: *mut efi::SystemTable) {
        unsafe {
            PRINTER.cout.store((*st).con_out, Ordering::Release);
        }
    }

    pub fn print(&self, string: &str) {
        let Some(cout) = (unsafe {PRINTER.cout.load(Ordering::Acquire).as_mut()}) else {
            return;
        };

        let mut string_u16: Vec<u16> = string.encode_utf16().collect();
        string_u16.push(0);
        unsafe {
            ((*cout).output_string)(cout, string_u16.as_mut_ptr());
        }
    }

    pub fn clrscr(&self) {
        let Some(cout) = (unsafe {PRINTER.cout.load(Ordering::Acquire).as_mut()}) else {
            return;
        };
        unsafe {
            ((*cout).clear_screen)(cout);
        }
    }
}
