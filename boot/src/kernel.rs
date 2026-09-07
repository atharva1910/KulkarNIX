use crate::{
    boot_ctx::BOOT_CTX, elfheader::{ELF_MAGIC, Elf64Dyn, Elf64Ehdr, Elf64Phdr, Elf64Rela, Elf64Shdr, PT_DYNAMIC, PT_LOAD, SHT_RELA}, file::EfiFile, printer::PRINTER
};
use r_efi::{
    efi::{self, ALLOCATE_ANY_PAGES, LOADER_DATA},
    protocols::{loaded_image, simple_file_system},
};
use alloc::{
    format, slice, vec::Vec
};

const PAGE_SIZE: u64 = 4096; // TODO make this usize
const KERNEL_VADDR: u64 = 0xfffffa0000000000;

pub struct Kernel {
    pub kernel_pages: usize,
    pub kernel_base: r_efi::base::PhysicalAddress,
    pub kernel_vaddr: u64,
    pub kernel_entry: u64,
}

fn vec_to_byte_slice<S>(input: &mut [S]) -> &mut [u8] {
     unsafe
    {
        slice::from_raw_parts_mut(
            input.as_mut_ptr().cast::<u8>(),
            input.len() * size_of::<S>(),
        )
    }
}

unsafe fn addr_to_byte_slice(input: u64, size: usize) -> &'static mut [u8] {
    unsafe {
        slice::from_raw_parts_mut(
            input as *mut u8,
            size
        )
    }
}

impl Kernel {
    pub fn new(h: efi::Handle) -> Result<Self, efi::Status> {
        let loaded_image =
            BOOT_CTX.handle_protocol::<loaded_image::Protocol>(h, loaded_image::PROTOCOL_GUID).ok_or(efi::Status::PROTOCOL_ERROR)?;
        let sfs =
            BOOT_CTX.handle_protocol::<simple_file_system::Protocol>(unsafe {(*loaded_image).device_handle},
                                                                     simple_file_system::PROTOCOL_GUID).ok_or(efi::Status::PROTOCOL_ERROR)?;

        let Some(fhandle) = EfiFile::open_handle(sfs, "\\Kernel.elf") else {
            return Err(efi::Status::ABORTED);
        };
        fhandle.seek(0);

        let mut elf_header: Elf64Ehdr = Elf64Ehdr::default();
        let mut status = fhandle.read_struct(&mut elf_header);
        if status != efi::Status::SUCCESS {
            return Err(status);
        }

        if elf_header.e_ident[..4] != ELF_MAGIC {
            PRINTER.print("ELF HEADER does not match\n");
            return Err(efi::Status::INVALID_PARAMETER);
        }

        fhandle.seek(elf_header.e_phoff as usize);

        let mut ph_buf = alloc::vec![ Elf64Phdr::default(); elf_header.e_phnum as usize];
        status = fhandle.read_bytes(vec_to_byte_slice::<Elf64Phdr>(&mut ph_buf));
        if status != efi::Status::SUCCESS {
            return Err(status);
        }

        let mut min_paddr = u64::MAX;
        let mut max_paddr = u64::MIN;

        for ph in &ph_buf {
            if ph.p_type != PT_LOAD || ph.p_memsz == 0 {
                continue;
            }

            if ph.p_paddr < min_paddr {
                min_paddr = ph.p_paddr;
            }

            if ph.p_memsz + ph.p_paddr > max_paddr {
                max_paddr = ph.p_memsz + ph.p_paddr;
            }
        }

        let total_size = max_paddr - min_paddr;
        let kernel_pages = ((total_size + (PAGE_SIZE - 1))/PAGE_SIZE) as usize;
        PRINTER.print(&format!("Total Size: 0x{:x} Total Pages : {:x} Min_Paddr: {:x} Max_Paddr: {:x}\n", total_size, kernel_pages, min_paddr, max_paddr));

        let Some(bs) = BOOT_CTX.get_bs() else {
            return Err(efi::Status::INVALID_PARAMETER);
        };

        let mut kernel_base: r_efi::base::PhysicalAddress = 0x0;
        status = unsafe {
            (bs.allocate_pages)(ALLOCATE_ANY_PAGES, LOADER_DATA, kernel_pages as usize, &mut kernel_base)
        };
        if status != efi::Status::SUCCESS {
            return Err(status);
        }

        let kbuffer = unsafe {
            core::slice::from_raw_parts_mut(kernel_base as *mut u8, kernel_pages << 12)
        };
        kbuffer.fill(0);

        for ph in &ph_buf {
            if ph.p_type != PT_LOAD || ph.p_memsz == 0 {
                continue;
            }

            fhandle.seek(ph.p_offset as usize);
            let start = (ph.p_paddr - min_paddr) as usize;
            let end = start + ph.p_memsz as usize;

            PRINTER.print(&format!("pgram header eloaded at start 0x{:x} end {:x}\n", start as u64 + min_paddr, end as u64 + min_paddr));
            if ph.p_type != PT_LOAD || ph.p_memsz == 0 {
                continue;
            }

            status = fhandle.read_bytes(&mut kbuffer[start..end]);
            if status != efi::Status::SUCCESS {
                PRINTER.print("Failed to load pgram header\n");
                return Err(status);

            }
        }

        for ph in ph_buf.iter() {
            if ph.p_type != PT_DYNAMIC {
                continue;
            }

            let mut dyn_arr_start = (ph.p_vaddr - KERNEL_VADDR + kernel_base) as usize;
            PRINTER.print(&format!("PT_DYNAMIC vaddr: {:x}  kernel_base: {:x} dyn_arr: {:x}\n", ph.p_vaddr,  kernel_base, dyn_arr_start));
            let mut rela_addr = 0;
            let mut rela_size = 0;
            let mut rela_count = 0;
            loop {
                let dyn_arr = dyn_arr_start as *const Elf64Dyn;

                unsafe {
                    match (*dyn_arr).d_tag {
                        7 => rela_addr = (*dyn_arr).d_val - KERNEL_VADDR + kernel_base,
                        8 => rela_size = (*dyn_arr).d_val,
                        0 => break,
                        _ => {},
                    }
                }
                dyn_arr_start += size_of::<Elf64Dyn>();
            }

            rela_count = rela_size as usize/size_of::<Elf64Rela>();
            PRINTER.print(&format!("rela_addr: {:x} rela_size: {:x} rela_count: {}\n", rela_addr, rela_size, rela_count));

            let rela_arr = unsafe {
                core::slice::from_raw_parts(rela_addr as *const Elf64Rela, rela_count)
            };

            for rela in rela_arr {
                PRINTER.print(&format!("rela r_offset: {:x} r_info: {:x} r_append: {:x}\n", rela.r_offset, rela.r_info, rela.r_append));
            }

        }
        PRINTER.print(&format!("Kernel loaded at: {:x} Kernel Entry: 0x{:x}\n", kernel_base, elf_header.e_entry));


        Ok(Self {
            kernel_pages: kernel_pages as usize,
            kernel_base,
            kernel_vaddr: KERNEL_VADDR,
            kernel_entry: elf_header.e_entry})
        }
    }
