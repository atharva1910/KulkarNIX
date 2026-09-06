use crate::{
    boot_ctx::BOOT_CTX, elfheader::{ELF_MAGIC, Elf64Ehdr, Elf64Phdr, Elf64Rela, Elf64Shdr, PT_LOAD, SHT_RELA}, file::EfiFile, printer::PRINTER
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

fn vec_to_byte<S>(input: &mut [S]) -> &mut [u8] {
     unsafe
    {
        slice::from_raw_parts_mut(
            input.as_mut_ptr().cast::<u8>(),
            input.len() * size_of::<S>(),
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
        status = fhandle.read_bytes(vec_to_byte::<Elf64Phdr>(&mut ph_buf));
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

        PRINTER.print(&format!("Kernel loaded at: {:x} Kernel Entry: 0x{:x}\n", kernel_base, elf_header.e_entry));

        // Read section Header
        fhandle.seek(elf_header.e_shoff as usize);
        let sh_size =  elf_header.e_shentsize * elf_header.e_shnum;

        let mut sh_buf = alloc::vec![ Elf64Shdr::default(); elf_header.e_shnum as usize];
        let x = unsafe {
            core::slice::from_raw_parts_mut(sh_buf.as_mut_ptr().cast::<u8>(),sh_size as usize)
        };
        status = fhandle.read_bytes(vec_to_byte::<Elf64Shdr>(&mut sh_buf));
        if status != efi::Status::SUCCESS {
            return Err(status);
        }

        for sh in sh_buf.iter() {
            if sh.sh_type != SHT_RELA {
                continue;
            }

            fhandle.seek(sh.sh_offset as usize);
            let mut rela_buf = alloc::vec![ Elf64Rela::default(); sh.sh_size as usize/size_of::<Elf64Rela>()];
            status = fhandle.read_bytes(vec_to_byte::<Elf64Rela>(&mut rela_buf));
            if status != efi::Status::SUCCESS {
                PRINTER.print("Failed to read section relocation bytes\n");
                return Err(status);
            }

            let target = &sh_buf[sh.sh_info as usize];
            for rela in rela_buf.iter() {
                let r_type = (rela.r_info & 0xFFFF_FFFF) as u32;
                let r_sym = (rela.r_info >> 32) as u32 & 0xFFFF_FFFF;
                let patch = target.sh_addr + rela.r_offset;
            }
        }

        Ok(Self {
            kernel_pages: kernel_pages as usize,
            kernel_base,
            kernel_vaddr: KERNEL_VADDR,
            kernel_entry: elf_header.e_entry})
        }
    }
