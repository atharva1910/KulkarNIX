    .text
    .global SetupPagingAsm
SetupPagingAsm:
    pusha
    # Point cr3 to the PML4 Table base
    movl $0x100000, %ecx
    movl %ecx, %cr3

    # Enable PAE at cr4
    movl %cr4, %eax
    orl  $(1<<5), %eax
    movl %eax, %cr4

    # Setup Long mode
    movl $0xC0000080, %ecx
    rdmsr
    orl $(1<<8), %eax
    wrmsr

    # Enable Paging
    movl %cr0, %eax
    orl $(1<<31), %eax
    movl %eax, %cr0

    popa
    ret
