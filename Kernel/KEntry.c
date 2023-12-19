#include <KEntry.h>

__asm__(
    /* Set up the global function __start */
    ".section .text\n"
    ".global __start\n"
    ".type   __start, @function\n"

"__start:\n"
    "cli\n"
    "mov $0x0, %eax\n"
    "mov %ax, %ds\n"
    "mov %ax, %ss\n"
    "mov %ax, %es\n"
    "mov $stack_top, %esp\n"
    "call kernel_main\n"
    "ret\n"
    "hlt\n"

    /* Set up the stack area */
    ".section .bss\n"
    ".align 16\n"

"stack_bottom:\n"
    ".skip 16384\n"
"stack_top:  \n"

    /* Restore the data section */
    ".section .text\n"
);

#if 0
void
InitInterrupts()
{
    PIC pic;
    //pic.Remap8259();
    pic.SetupInterrupts();
    HAL::EnableInterrupts();
}

void
SetupPaging()
{
    SetupX86Paging();
}
#endif

void kernel_main()
{
    for(int i = 0; i < 25; i++)
        KPrint(KVERBOSE, "aaaaaaaaa ");

    asm("hlt");
}
