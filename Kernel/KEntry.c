#include "KInclude.h"

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
    "call KMain\n"
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

typedef struct _MMapEntry {
    uint32_t addr_low;
    uint32_t addr_high;
    uint32_t len_low;
    uint32_t len_high;            
    uint32_t type;
    uint32_t acpi;    
}MMapEntry, *PMMapEntry;

void KMain()
{
    PMMapEntry head = (PMMapEntry)0x7E00;
    for (int i = 0; i < 10; i++) {
        KPrint(KVERB, "[%d] Address: 0x%x Length: 0x%x", i, head->addr_low, head->len_low);
        head++;
    }
    KPrint(KVERB,"TEST:");
    asm("hlt");
}
