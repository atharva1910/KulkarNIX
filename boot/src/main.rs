#![no_main]
#![no_std]
mod boot_ctx;
mod kernel;
mod printer;
mod elfheader;
extern crate alloc;

use r_efi::efi;
use boot_ctx::BOOT_CTX;
use kernel::Kernel;
use crate::printer::PRINTER;

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
        return efi::Status::INVALID_PARAMETER;
    };
    efi::Status::SUCCESS
}
