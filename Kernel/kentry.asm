    .section .text
    .global __start
    .type   __start, @function

__start:
    # We have jumped here from the bootloader
    # Set back the segment registers, set up the stack
    # call C++ code
    cli
    mov     $stack_top, %esp
    push    $0x9000             #Memory map pointer
    call    kernel_main
    hlt
    hlt

    .section .bss
    .align 16
stack_bottom:
    .skip 16384
stack_top:  
