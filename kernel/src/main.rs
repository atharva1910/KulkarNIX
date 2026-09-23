#![no_std]
#![no_main]
mod serial_port;
mod errors;
mod hal;
mod heap_manager;
mod pmem_manager;
use core::arch::global_asm;
use pmem_manager::PMemManager;
//use core::fmt::Write;
use core::panic::PanicInfo;
use common::{KernelArgs, address::{PhysicalAddress, VirtualAddress}};

use crate::heap_manager::HeapManager;
//use serial_port::SerialPort;

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
pub extern "C" fn kernel_main(addr: PhysicalAddress) {
    let Ok(mut pmm) =  PMemManager::init(addr) else {
        serial_port::write("pmem_manager init successful\n");
        return; //hang
    };

    let heap_manager = HeapManager::init(&mut pmm);
    loop {};
}
