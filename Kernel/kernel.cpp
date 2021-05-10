#include "typedefs.h"

static inline void
PrintChar(char *address, char c, BYTE bg_color)
{
    address[1] = bg_color;
    address[0] = c;
}


static inline void
PrintString(char *string)
{
    char *vga_buffer = (char *)0xb8000;
    char c = 0;
    uint32_t pos = 0;
    while((c = string[pos++]) != '\0'){
        PrintChar(vga_buffer, c, 0x07);
        vga_buffer += 2;
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

extern "C"
void kernel_main()
{
    asm("hlt");
}

__asm__(
        /* Set up the global function __start */
    ".section .text\n"
    ".global __start\n"
    ".type   __start, @function\n"

"__start:\n"
"mov     $stack_top, %esp\n"
"call    kernel_main\n"
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
