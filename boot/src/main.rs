#![no_main]
#![no_std]
mod boot_ctx;
mod kernel;
mod printer;
mod file;
mod elfheader;
mod memory_map;
mod paging;

extern crate alloc;
use alloc::format;
use r_efi::{ efi::{self, ALLOCATE_ANY_PAGES, LOADER_CODE}, protocols::loaded_image};
use boot_ctx::BOOT_CTX;
use kernel::Kernel;
use crate::{
    memory_map::MemoryMap,
    paging::PageTableManager,
    printer::PRINTER
};

const ONE_KB: usize = 1024;
const ONE_MB: usize = ONE_KB * 1024;
const ONE_GB: usize = ONE_MB * 1024;
const PAGE_SIZE: usize = 4096; //1 << 12;

#[panic_handler]
fn panic_handler(_info: &core::panic::PanicInfo) -> ! {
    loop {}
}

fn page_allocator(num_pages: usize) -> Option<u64> {
    let Some(bs) = BOOT_CTX.get_bs() else {
        return None;
    };

    let mut paddr: r_efi::base::PhysicalAddress = 0;
    let status =
    unsafe {
        (bs.allocate_pages)(ALLOCATE_ANY_PAGES, LOADER_CODE, num_pages, &mut paddr)
    };
    if status != efi::Status::SUCCESS {
        return None;
    }

    unsafe {
        (bs.set_mem)(paddr as *mut core::ffi::c_void, PAGE_SIZE, 0);
    };

    Some(paddr)
}

fn setup_mem_paging<T>(pt_mgr: &PageTableManager<T>, mem_map: &MemoryMap)  ->  Result<(), efi::Status>
where
    T: Fn(usize) -> Option<u64>
{
    let total_mem = mem_map.total_memory;
    let num_1gb_pdpe = (total_mem + (ONE_GB - 1)) / ONE_GB;

    assert!(total_mem < 512 * ONE_GB);
    assert!(num_1gb_pdpe < paging::PAGE_TABLE_NUM_ENTRIES);
    PRINTER.print(&format!("num_1gb_pdpe: {:x}\n", num_1gb_pdpe));

    let Some(pml4t) = pt_mgr.get_pml4t_mut() else {
        return Err(efi::Status::INVALID_PARAMETER);
    };

    if pml4t.pml4e[256].is_entry_present() {
        assert!(false, "Entry already present");
    }

    let Some(pdpt) = pt_mgr.allocate_tables(1) else {
        return Err(efi::Status::OUT_OF_RESOURCES);
    };

    pml4t.pml4e[256].set_present();
    pml4t.pml4e[256].set_rw();
    pml4t.pml4e[256].set_addr(pdpt);


    let Some(pdpt) = (unsafe {
        (pdpt as *mut paging::PDPT).as_mut()
    }) else {
        return Err(efi::Status::INVALID_PARAMETER);
    };

    let mut addr = 0x0;
    for i in 0..num_1gb_pdpe {
        pdpt.set_1gb_paging(i, addr);
        addr += ONE_GB as u64;
    }

    Ok(())
}

fn setup_kernel_paging<T>(pt_mgr: &PageTableManager<T>, kernel: &Kernel)  ->  Result<(), efi::Status>
where
    T: Fn(usize) -> Option<u64>
{
    let mut start_paddr = kernel.kernel_base;
    let mut start_vaddr = kernel.kernel_vaddr;
    for _ in 0..kernel.kernel_pages {
        if pt_mgr.map_page(start_vaddr, start_paddr) {
            start_vaddr += PAGE_SIZE as u64;
            start_paddr += PAGE_SIZE as u64;
        } else {
            PRINTER.print("FAILED TO MAP KERNEL\n");
            return Err(efi::Status::OUT_OF_RESOURCES);
        }
    }

    Ok(())
}

fn setup_image_identity_map<T>(h: efi::Handle, pt_mgr: &PageTableManager<T>) -> Result<(), efi::Status>
where
    T: Fn(usize) -> Option<u64> {
    let loaded_image =
        BOOT_CTX.handle_protocol::<loaded_image::Protocol>(h, loaded_image::PROTOCOL_GUID).ok_or(efi::Status::PROTOCOL_ERROR)?;

    let mut image_base = unsafe {
        (*loaded_image).image_base as u64
    };

    let image_size = unsafe {
        (*loaded_image).image_size as u64
    };

    let pages = image_size >> 12;

    for _ in 0..pages {
        pt_mgr.map_page(image_base, image_base);
        image_base += PAGE_SIZE as u64;
    }
    Ok(())
}


fn setup_paging(h: efi::Handle, kernel: &Kernel, mem_map: &MemoryMap) -> Result<PageTableManager<impl Fn(usize) -> Option<u64>>, efi::Status> {
    let Some(pt_mgr) = PageTableManager::new(page_allocator) else {
        PRINTER.print("setup_paging failed");
        return  Err(efi::Status::INVALID_PARAMETER);
    };

    setup_mem_paging(&pt_mgr, mem_map)?;
    setup_kernel_paging(&pt_mgr, kernel)?;
    setup_image_identity_map(h, &pt_mgr)?;
    Ok(pt_mgr)
}

#[unsafe(export_name = "efi_main")]
pub extern "efiapi" fn main(h: efi::Handle,
                            st: *mut efi::SystemTable) -> efi::Status {

    BOOT_CTX.new(st);
    PRINTER.init(st);
    PRINTER.clrscr();

    let Some(bs) = BOOT_CTX.get_bs() else {
        return BOOT_CTX.halt();
    };

    unsafe {
        (bs.set_watchdog_timer)(0,0,0,core::ptr::null_mut());
    }

    let Ok(kernel)= Kernel::new(h) else {
        PRINTER.print("Kernel setup failed");
        return BOOT_CTX.halt();
    };

    let Ok(mem_map) = MemoryMap::new() else {
        PRINTER.print("Failed to get memory map");
        return BOOT_CTX.halt();
    };

    let pt_mgr = match setup_paging(h, &kernel, &mem_map) {
        Ok(x) => x,
        Err(s) => return s,
    };

    let Some(pml4t) = pt_mgr.get_pml4t() else {
        PRINTER.print("PML4 not setup!?");
        return BOOT_CTX.halt();
    };

    PRINTER.print(&format!("Jumping to Kernel at : {}", kernel.kernel_vaddr));

    let Ok(mem_map) = MemoryMap::new() else {
        PRINTER.print("Failed to get memory map");
        return BOOT_CTX.halt();
    };

    // TODO make this a retry-loop
    let status = unsafe {
        (bs.exit_boot_services)(h, mem_map.key)
    };
    if status != efi::Status::SUCCESS {
        PRINTER.print("Failed to exit boot services");
        return BOOT_CTX.halt();
    }
    unsafe {
        core::arch::asm!(
            "cli",
            "mov cr3, {}",
            "jmp {entry}",
            in(reg) pml4t,
            entry = in(reg) kernel.kernel_vaddr,
        );
    }

    BOOT_CTX.halt()
}
