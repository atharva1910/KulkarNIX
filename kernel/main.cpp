#include "KulkarNIX.h"
#include "PMemManager.h"

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


extern "C"
void main(void *args)
{
    Logger logger;
    logger.print("Welcome to the kernel :)");

    auto m_args = reinterpret_cast<KernelArgs*>(PA2VA<void*>(args));
    PMemManager mm(logger);
    if (!mm.init(m_args)) {
        assert(logger, false, "Failed to init PMemManager");
    }
}
