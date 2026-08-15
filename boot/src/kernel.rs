use core::ptr::slice_from_raw_parts_mut;
use crate::{
    boot_ctx::BOOT_CTX,
    elfheader::{ELF_MAGIC, Elf64Ehdr, Elf64Phdr}, file::EfiFile,
    printer::PRINTER,
};
use r_efi::{
    efi::{self, ALLOCATE_ANY_PAGES, LOADER_DATA},
    protocols::{loaded_image, simple_file_system},
};
use alloc::{
    format,
    vec::Vec,
};

const PAGE_SIZE: u64 = 4096; // TODO make this usize
pub struct Kernel {
    kernel_pages: usize,
    kernel_base: r_efi::base::PhysicalAddress,
}

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

        let Some(fhandle) = EfiFile::open_handle(sfs, "\\Kernel.elf") else {
            return Err(efi::Status::ABORTED);
        };
        fhandle.seek(0);

        let mut elf_header: Elf64Ehdr = Elf64Ehdr::default();
        status = fhandle.read_struct(&mut elf_header);
        if status != efi::Status::SUCCESS {
            return Err(status);
        }

        if elf_header.e_ident[..4] != ELF_MAGIC {
            PRINTER.print("ELF HEADER does not match\n");
            return Err(efi::Status::INVALID_PARAMETER);
        }

        fhandle.seek(elf_header.e_phoff as usize);

        let ph_size =  elf_header.e_phentsize * elf_header.e_phnum;
        let mut ph_buf = alloc::vec![ Elf64Phdr::default(); elf_header.e_phnum as usize];
        let x = unsafe {
            core::slice::from_raw_parts_mut(ph_buf.as_mut_ptr().cast::<u8>(), ph_size as usize)
        };
        status = fhandle.read_bytes(x);
        if status != efi::Status::SUCCESS {
            return Err(status);
        }

        let mut total_size = 0;
        for ph in &ph_buf {
            total_size += ph.p_memsz;
        }
        let kernel_pages = ((total_size + (PAGE_SIZE - 1))/PAGE_SIZE) as usize;
        PRINTER.print(&format!("Total Pages : {}\n", kernel_pages));

        let Some(bs) = BOOT_CTX.get_bs() else {
            return Err(efi::Status::INVALID_PARAMETER);
        };

        let mut kernel_base: r_efi::base::PhysicalAddress = 0x0;
        status = unsafe {
            (bs.allocate_pages)(ALLOCATE_ANY_PAGES, LOADER_DATA, kernel_pages as usize, &mut kernel_base)
        };
        if status != efi::Status::SUCCESS {
            return Err(status);
        } else {
            PRINTER.print(&format!("Kernel Base allocated at: {:x}\n", kernel_base));
        }

        let kbuffer = unsafe {
            core::slice::from_raw_parts_mut(kernel_base as *mut u8, kernel_pages << 12)
        };

        let mut start:usize = 0;
        for ph in &ph_buf {
            fhandle.seek(ph.p_offset as usize);
            let end = start + ph.p_offset as usize;
            status = fhandle.read_bytes(&mut kbuffer[start..end]);
            if status != efi::Status::SUCCESS {
                return Err(status);
            }
            start = end;
        }
        Ok(Self {
            kernel_pages: kernel_pages as usize,
            kernel_base: kernel_base,
        })
        }
    }
