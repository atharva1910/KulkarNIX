    bits 32
    section .text
    global __start
    extern boot_main
__start:
    ;; We have jumped here from the bootloader
    ;; Set back the segment registers, set up the stack
    ;; call C++ code
    ;xor     eax, eax
    ;mov     ds, eax
    ;mov     ss, eax
    ;mov     es, eax
    cli
    pop     ax
    mov     ds, ax
    mov     ss, ax
    mov     es, ax
    mov     sp, 07c00h
    call    boot_main
    hlt
    hlt

%if 0
    global PPrintString
PPrintString:   
    pusha
    mov     ebx, 0b8000h
    xor     cx, cx
.loop:
    lodsb
    cmp     al, 0
    je      .end
    mov     ah, 07h             ;black baackground, white foreground
    mov     word [ebx], ax
    add     bx, 02h
    jmp     .loop
.end:
    popa
    ret

    section .data
WelcomeString: db "Welcome to the Kernel",0
TestSting: db "After read",0
%endif
