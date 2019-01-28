;.section .text
;.global _start
    bits    32
_start:
    ;; We have jumped here from the bootloader
    ;; Set back the segment registers, set up the stack
    ;; call C++ code
    pop     eax
    mov     ds, eax
    mov     cs, eax
    mov     ss, eax
    mov     es, eax
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
WelcomeString: db "Hello from the jump",0


times 512 - ($-$$) db 0
    
    
