use r_efi::efi;
use crate::{
    boot_ctx::BOOT_CTX,
    elfheader::{ELF_MAGIC, Elf64Ehdr}, file::EfiFile,
};
use r_efi::protocols::{loaded_image, simple_file_system};
use alloc::format;
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

        // Get the sfs protocol to read the file
        guid = simple_file_system::PROTOCOL_GUID;
        p = core::ptr::null_mut();
        status = unsafe { (bs.handle_protocol)((*loaded_image).device_handle, &mut guid, &mut p) };
        if status != efi::Status::SUCCESS {
            return Err(status);
        }
        let sfs: *mut simple_file_system::Protocol = p.cast();
        PRINTER.print("got the sfs\n");

        let Some(fHandle) = EfiFile::open_handle(sfs, "\\Kernel.elf") else {
            PRINTER.print("got the sfs\n");
            return Err(efi::Status::ABORTED);
        };

        fHandle.seek(0);

        let mut elf_header: Elf64Ehdr = Elf64Ehdr::default();
        let mut buf_size: usize = size_of::<Elf64Ehdr>();
        status = unsafe {((*khandle).read)(fhandle, &mut buf_size, &mut elf_header as * mut _ as *mut core::ffi::c_void)};
        if status != efi::Status::SUCCESS {
            PRINTER.print(&format!("file read failed: {} {}\n", status, buf_size));
            loop{};
            return Err(status);
        }
        PRINTER.print("read succ\n");

        if elf_header.e_ident[..4] == ELF_MAGIC {
            let Some(st) =  BOOT_CTX.get_st() else {
                return Err(status);
            };
            unsafe {((*st.con_out).clear_screen)(st.con_out);}
        }
        Ok(Self {})
        }
    }
