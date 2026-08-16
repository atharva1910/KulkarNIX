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
use r_efi::efi;
use boot_ctx::BOOT_CTX;
use kernel::Kernel;
use crate::{memory_map::MemoryMap, printer::PRINTER};

#[panic_handler]
fn panic_handler(_info: &core::panic::PanicInfo) -> ! {
    loop {}
}

#[unsafe(export_name = "efi_main")]
pub extern "efiapi" fn main(h: efi::Handle,
                            st: *mut efi::SystemTable) -> efi::Status {

    BOOT_CTX.new(st);
    PRINTER.init(st);
    PRINTER.clrscr();

    let Ok(kernel)= Kernel::new(h) else {
        PRINTER.print("Kernel setup failed");
        return BOOT_CTX.halt();
    };

    let Ok(mem_map) = MemoryMap::new() else {
        PRINTER.print("Failed to get memory map");
        return BOOT_CTX.halt();
    };
    PRINTER.print(        &format!("Kernel: {:x} pages {:x}", kernel.kernel_base, kernel.kernel_pages));
    PRINTER.print(        &format!("Memory: {:x}", mem_map.total_memory));
    efi::Status::SUCCESS
}
