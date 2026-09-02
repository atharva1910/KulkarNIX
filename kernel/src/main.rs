#![no_std]
#![no_main]
mod serial_port;
mod errors;
mod hal;
use core::arch::global_asm;
use core::fmt::Write;
use core::panic::PanicInfo;
use common::KernelArgs;
use r_efi::efi::MemoryDescriptor;
use common::serial_port::SERIAL_PORT;

use crate::serial_port::SerialPort;

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}

global_asm!(
    ".section .text",
    ".global __start",
    "__start:",
    "cli",
    "lea rsp, [rip + stack_top]",
    "call kernel_main",

    "hang:",
    "hlt",
    "jmp hang",

    /* Setup the stack */
    ".section .bss",
    ".align 16",
    "stack_bottom:",
    ".skip 0x4000",
    "stack_top:",

    /* Restore the data section */
    ".section .text\n",
);

#[unsafe(no_mangle)]
pub extern "C" fn kernel_main(addr: u64) {
    let mut s = SerialPort{ test: 42};
    write!(s, "test 0x{:x}\n", addr);
    loop {};
}
