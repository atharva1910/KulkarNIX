.section .text
    .global _start
_start:
    ;; We have jumped here from the bootloader
    ;; Set back the segment registers, set up the stack
    ;; call C++ code

PPrintString:   
    pusha
    mov     si, ProtectedModeWelcomeString
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
ProtectedModeWelcomeString: db "Welcome to protected mode",0


    
    
