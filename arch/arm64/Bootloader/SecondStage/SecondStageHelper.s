#; Access bits
#;PRESENT        equ 1 << 7
#;NOT_SYS        equ 1 << 4
#;EXEC           equ 1 << 3
#;DC             equ 1 << 2
#;RW             equ 1 << 1
#;ACCESSED       equ 1 << 0
#;
#;; Flags bits
#;GRAN_4K       equ 1 << 7
#;SZ_32         equ 1 << 6
#;    LONG_MODE     equ 1 << 5

    .data
GDT:
    .quad 0
Code:
    .long 0xFFFF
    .byte 0
    .byte ((1 << 7) | (1 << 4) | (1 << 3) | (1 << 1))
    .byte ((1 << 7) | (1 << 5) | 0xF)
    .byte 0
Data:
    .long 0xFFFF
    .byte 0
    .byte ((1 << 7) | (1 << 4) | (1 << 1))
    .byte ((1 << 7) | (1 << 6) | 0xF)
    .byte 0
TSS:
    .long 0x00000068
    .long 0x00CF8900
Pointer:
    .word . - GDT - 1
    .long GDT


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

    .global LoadGDTAsm
LoadGDTAsm:
    pusha
    lgdt Pointer
    popa
    ret
