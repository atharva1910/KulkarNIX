#![no_main]
#![no_std]
mod boot_ctx;
mod kernel;
mod printer;
mod file;
mod elfheader;
mod memory_map;

extern crate alloc;
use alloc::format;
use r_efi::{ efi::{self, ALLOCATE_ANY_PAGES, LOADER_CODE}, protocols::{graphics_output, loaded_image}};
use boot_ctx::BOOT_CTX;
use kernel::Kernel;
use common::{
    KERNEL_ARGS_PAGES, KERNEL_DS_ADDR, KernelArgs, address::{PhysicalAddress, VirtualAddress}, paging::{self, PageTableManager}, serial_port
};
use crate::{
    memory_map::MemoryMap,
    serial_port::SerialPort
};

const ONE_KB: usize = 1024;
const ONE_MB: usize = ONE_KB * 1024;
const ONE_GB: usize = ONE_MB * 1024;
const PAGE_SIZE: usize = 4096; //1 << 12;

#[panic_handler]
fn panic_handler(_info: &core::panic::PanicInfo) -> ! {
    loop {}
}

fn page_allocator(num_pages: usize) -> Option<usize> {
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

    Some(paddr as usize)
}

fn prepare_kernel_args(paddr: usize, mem_map: &MemoryMap) -> Option<()> {
    let gop = BOOT_CTX.locate_protocol::<graphics_output::Protocol>(graphics_output::PROTOCOL_GUID)?;
    let mode = unsafe {(*gop).mode.as_ref()}?;

    let args = unsafe {
        (paddr as *mut KernelArgs).as_mut()?
    };

    args.desc_size = mem_map.desc_size;
    args.mem_map_size = mem_map.mem_map_size;

    if args.buffer.len() < mem_map.buffer.len() {
        return None;
    }

    args.buffer[..mem_map.buffer.len()].copy_from_slice(&mem_map.buffer[..]);
    args.frame_buf_info.mode_information = unsafe {
        mode.info.as_ref().unwrap().clone()
    };
    args.frame_buf_info.frame_base =  mode.frame_buffer_base;
    args.frame_buf_info.frame_size =  mode.frame_buffer_size;
    Some(())
}


fn setup_mem_paging<T>(pt_mgr: &PageTableManager<T>, mem_map: &MemoryMap)  ->  Result<(), efi::Status>
where
    T: Fn(usize) -> Option<usize>
{
    let total_mem = mem_map.total_memory;
    let num_1gb_pdpe = (total_mem + (ONE_GB - 1)) / ONE_GB;

    assert!(total_mem < 512 * ONE_GB);
    assert!(num_1gb_pdpe < paging::PAGE_TABLE_NUM_ENTRIES);

    let mut paddr = PhysicalAddress(0x0);
    let mut vaddr = VirtualAddress(KERNEL_DS_ADDR);
    for _ in 0..num_1gb_pdpe {
        pt_mgr.map_1gb_page(paddr, vaddr);
        paddr += ONE_GB as usize;
        vaddr += ONE_GB as usize;
    }

    Ok(())
}

fn setup_kernel_paging<T>(pt_mgr: &PageTableManager<T>, kernel: &Kernel)  ->  Result<(), efi::Status>
where
    T: Fn(usize) -> Option<usize>
{
    let mut start_paddr = kernel.kernel_base;
    let mut start_vaddr = kernel.kernel_vaddr;
    for _ in 0..kernel.kernel_pages {
        if pt_mgr.map_page( start_paddr.get_raw(), start_vaddr.get_raw()) {
            start_vaddr += PAGE_SIZE;
            start_paddr += PAGE_SIZE;
        } else {
            printer::print("FAILED TO MAP KERNEL\n");
            return Err(efi::Status::OUT_OF_RESOURCES);
        }
    }

    Ok(())
}

fn setup_image_identity_map<T>(h: efi::Handle, pt_mgr: &PageTableManager<T>) -> Result<(), efi::Status>
where
    T: Fn(usize) -> Option<usize> {
    let loaded_image =
        BOOT_CTX.handle_protocol::<loaded_image::Protocol>(h, loaded_image::PROTOCOL_GUID).ok_or(efi::Status::PROTOCOL_ERROR)?;

    let mut image_base = unsafe {
        (*loaded_image).image_base as usize
    };

    let image_size = unsafe {
        (*loaded_image).image_size as usize
    };

    let pages = image_size >> 12;

    printer::print(&format!("Mapping image. Base 0x{:x} Size 0x{:x}\n", image_base, image_size));
    for _ in 0..pages {
        pt_mgr.map_page(image_base, image_base);
        image_base += PAGE_SIZE;
    }
    Ok(())
}


fn setup_paging(h: efi::Handle, kernel: &Kernel, mem_map: &MemoryMap) -> Result<PageTableManager<impl Fn(usize) -> Option<usize>>, efi::Status> {
    let Some(pt_mgr) = PageTableManager::new(page_allocator) else {
        printer::print("setup_paging failed");
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
    printer::init(st);
    printer::clrscr();
    SerialPort::init();

    let Some(bs) = BOOT_CTX.get_bs() else {
        return BOOT_CTX.halt();
    };

    unsafe {
        (bs.set_watchdog_timer)(0,0,0,core::ptr::null_mut());
    }

    let Ok(kernel)= Kernel::new(h) else {
        printer::print("Kernel setup failed");
        return BOOT_CTX.halt();
    };

    let Ok(mem_map) = MemoryMap::new() else {
        printer::print("Failed to get memory map");
        return BOOT_CTX.halt();
    };

    let pt_mgr = match setup_paging(h, &kernel, &mem_map) {
        Ok(x) => x,
        Err(s) => return s,
    };

    let Some(pml4t) = pt_mgr.get_pml4t() else {
        printer::print("PML4 not setup!?");
        return BOOT_CTX.halt();
    };


    let Some(paddr) = page_allocator(KERNEL_ARGS_PAGES) else {
        return BOOT_CTX.halt();
    };

    printer::print(&format!("Jumping to Kernel at : 0x{:x}. Args 0x{:x}\n", kernel.kernel_entry, paddr));

	let Ok(mem_map) = MemoryMap::new() else {
	    printer::print("Failed to get memory map\n");
	    return BOOT_CTX.halt();
	};

    if prepare_kernel_args(paddr, &mem_map).is_none() {
	    printer::print("Failed to setup kernel args\n");
	    return BOOT_CTX.halt();
    }

	let status = unsafe {
	    (bs.exit_boot_services)(h, mem_map.key)
	};
	if status != efi::Status::SUCCESS {
	    printer::print(&format!("Failed to exit boot services {}\n", status));
        return BOOT_CTX.halt();
    }

    unsafe {
        core::arch::asm!(
            "cli",
            "mov cr3, {pml4}",
            "jmp {entry}",
            in("rdi") paddr,
            pml4 = in(reg) pml4t as *const _ as u64,
            entry = in(reg) kernel.kernel_entry.get_raw(),
            options(noreturn)
        );
    }
}
