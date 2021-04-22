#include "typedefs.h"
#include "HAL/HAL.h"
#include "Debug/Debug.h"

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
    //print_string("Setting up paging for x86");
    SetupX86Paging();
}

extern "C"
void kernel_main(void *memory_map)
{
    const char *c = "Hello from Kernel";
    print_string(c);
    //HAL::DisableInterrupts();
    //SetupPaging();
    //InitInterrupts();
    while(1);
}

__asm__(
        /* Set up the global function __start */
    ".section .text\n"
    ".global __start\n"
    ".type   __start, @function\n"

"__start:\n"
"mov     $stack_top, %esp\n"
"push    $0x9000             #Memory map pointer\n"
"call    kernel_main\n"
"hlt\n"
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
