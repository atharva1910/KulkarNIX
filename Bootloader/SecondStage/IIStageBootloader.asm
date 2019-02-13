    bits    32
    extern  boot_main

    section .text
    global  __start
__start:
    ;; We have jumped here from the bootloader
    ;; Set back the segment registers, set up the stack
    ;; call C++ code
    ;xor     eax, eax
    ;mov     ds, eax
    ;mov     ss, eax
    ;mov     es, eax
    pop     ax
    mov     ds, ax
    mov     ss, ax
    mov     es, ax
    mov     sp, 07c00h
    call    boot_main
    hlt
    hlt
    hlt

    global ata_bsy_wait
ata_bsy_wait:
    pusha
    xor al, al
    mov dx, 01F7h
    in  al, dx
    
.loop:   
    test al, 0EFh
    jnz .loop
.end:
    popa
    ret

    global ata_drq_wait
ata_drq_wait:
    pusha
    xor ax, ax
    mov dx, 01F7h
    in  al, dx
.loop:
    test al, 0FBh
    jnz .loop
.end:
    popa
    ret
    
    %if 0
    global PPrintString
PPrintString:   
    pusha
    mov     si, WelcomeString
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
    hlt
    hlt
    popa
    ret

    section .data
WelcomeString: db "Welcome to the Kernel",0
    %endif
