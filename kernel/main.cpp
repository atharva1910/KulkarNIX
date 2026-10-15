#include "KulkarNIX.h"
#include "Assert.h"
#include "Logger.h"
#include "PMemManager.h"
#include "SerialPort.h"
#include "HeapManager.h"

__asm__(
    /* Set up the global function __start */
    ".section .text\n"
    ".global __start\n"
    ".type   __start, @function\n"

"__start:\n"
    "cli\n"
    /*
    Cannot reset ss and es for i686. For amd64 it should be 0
    "mov $0x0, %eax\n"
    "mov %ax, %ds\n"
    "mov %ax, %ss\n"
    "mov %ax, %es\n"
    */
    "movabs $stack_top, %rsp\n"
    "call main\n"
    "hang:\n"
    "hlt\n"
    "jmp hang\n"
    "ret\n"

    /* Set up the stack area */
    ".section .bss\n"
    ".align 16\n"

"stack_bottom:\n"
    ".skip 16384\n"
"stack_top:  \n"

    /* Restore the data section */
    ".section .text\n"
);


extern "C" void main(PA args) {
    SerialPort::init();

    auto m_args = reinterpret_cast<KernelArgs*>(VA(args).get_raw());
    Logger::print("Welcome to the kernel ", m_args->magic);

    PMM::init(m_args);
    Heap::init();

    auto p = Heap::alloc(4096 * 4);
    Logger::print("Allocated ", p);
    assert(false, "End of kernel_main");
}
