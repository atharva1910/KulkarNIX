
#![no_std]
#![no_main]
use core::arch::global_asm;
use core::panic::PanicInfo;

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
    "hlt",

    ".section .bss\n",
    "stack_bottom:",
    ".skip 0x4000",
    "stack_top:",

    /* Restore the data section */
    ".section .text\n",
);

#[unsafe(no_mangle)]
pub extern "C" fn kernel_main() {
    loop{}
}
