#![no_std]
#![no_main]
mod serial_port;
mod errors;
mod hal;
mod heap_manager;
mod pmem_manager;
mod linked_list;
use core::fmt::{self, Write};
use core::panic::PanicInfo;
use core::arch::global_asm;
use pmem_manager::PMemManager;
use serial_port::SerialPort;
use common::{KernelArgs, address::{PhysicalAddress, VirtualAddress}};

use crate::heap_manager::HeapManager;
//use serial_port::SerialPort;

#[panic_handler]
fn panic(info: &PanicInfo) -> ! {
    SPrint!("PANIC");
    SPrint!("{}", info);
    SPrint!("PANIC");
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
    SPrint!("Welcome to the kernel PArgs: {:X} Vargs {:X}", addr, addr.to_virtual());

    let mut pmm =  PMemManager::init(addr.to_virtual()).unwrap();
    SPrint!("Physical Memory Manager init successful");

    let mut hmm = HeapManager::init(&mut pmm);

    if let Some(addr) = hmm.alloc(512) {
        SPrint!("Got address {}", *addr);
        serial_port::write("hmm init successful\n");
    }
    loop {};
}
