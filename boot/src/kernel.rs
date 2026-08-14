use r_efi::efi;
use crate::{
    boot_ctx::BOOT_CTX,
    elfheader::{ELF_MAGIC, Elf64Ehdr},
};
use r_efi::protocols::{loaded_image, simple_file_system, file};
use alloc::{
    vec::Vec,
    string::String,
    format,
};
use crate::printer::PRINTER;
pub struct Kernel;

impl Kernel {
    pub fn new(h: efi::Handle) -> Result<Self, efi::Status> {
        let bs = BOOT_CTX.get_bs().unwrap();

        // Get the loaded image protocol for the handle
        let mut guid = loaded_image::PROTOCOL_GUID;
        let mut p = core::ptr::null_mut();
        let mut status = unsafe { (bs.handle_protocol)(h, &mut guid, &mut p) };
        if status != efi::Status::SUCCESS {
            return Err(status);
        }

        let loaded_image: *mut loaded_image::Protocol = p.cast();
        PRINTER.print("got the loaded image\n");

        // Get the sfs protocol to read the file
        guid = simple_file_system::PROTOCOL_GUID;
        p = core::ptr::null_mut();
        status = unsafe { (bs.handle_protocol)((*loaded_image).device_handle, &mut guid, &mut p) };
        if status != efi::Status::SUCCESS {
            return Err(status);
        }
        let sfs: *mut simple_file_system::Protocol = p.cast();
        PRINTER.print("got the sfs\n");

        let mut fhandle: *mut file::Protocol = core::ptr::null_mut();
        status = unsafe {((*sfs).open_volume)(sfs, &mut fhandle)};
        if status != efi::Status::SUCCESS {
            return Err(status);
        }
        PRINTER.print("open vol succ\n");

        let mut khandle: *mut file::Protocol = core::ptr::null_mut();
        let mut file_name: Vec<u16> = "\\Kernel.bin".encode_utf16().collect();
        file_name.push(0);
        status = unsafe {((*fhandle).open)(fhandle, &mut khandle, file_name.as_mut_ptr(), 1, 1)};
        if status != efi::Status::SUCCESS {
            PRINTER.print(&format!("file open succ: {}\n", status));
            return Err(status);
        }
        PRINTER.print("file open succ\n");

        let mut elf_header: Elf64Ehdr = Elf64Ehdr::default();
        let mut buf_size: usize = size_of::<Elf64Ehdr>();
        status = unsafe {((*khandle).read)(khandle, &mut buf_size, &mut elf_header as * mut _ as *mut core::ffi::c_void)};
        if status != efi::Status::SUCCESS {
            return Err(status);
        }
        PRINTER.print("read succ\n");

        if elf_header.e_ident[..4] == ELF_MAGIC {
            let Some(st) =  BOOT_CTX.get_st() else {
                return Err(status);
            };
            unsafe {((*st.con_out).clear_screen)(st.con_out);}
            loop {};
        }
        Ok(Self {})
        }
    }
