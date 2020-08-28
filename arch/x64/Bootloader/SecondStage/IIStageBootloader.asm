    bits 32
    section .text
    global __start
    extern boot_main
__start:
    ;; We have jumped here from the bootloader
    ;; Set back the segment registers, set up the stack
    ;; call C code
    cli
    pop     ax
    mov     ds, ax
    mov     ss, ax
    mov     es, ax
    mov     sp, 07c00h
    call    boot_main
    hlt
    hlt
