#include <KEntry.h>

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

static inline void
PrintChar(char c, byte bg_color, uint32_t pos)
{
    char *address = (char *)0xB8000 + pos * 2;

    address[1] = bg_color;
    address[0] = c;
}

static void
PrintString(const char *string)
{
    char c = 0;
    uint32_t pos = 0;

    /* Print string to screen */
    while((c = string[pos]) != '\0') {
        PrintChar(c, 0x07, pos++);
    }

}

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
    KPrint(KVERB, "abc 0x%x def", 42);
    KPrint(KVERB, "abc 0x%x def", (uint32_t)(kernel_main));
    asm("hlt");
}
