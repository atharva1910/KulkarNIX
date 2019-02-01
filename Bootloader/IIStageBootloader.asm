    bits    32
    org     08000h
_start:
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
    call    PPrintString
    hlt
    hlt
    hlt

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
WelcomeString: db "Welcome to the Kernel",0


times 512 - ($-$$) db 0
    
    
