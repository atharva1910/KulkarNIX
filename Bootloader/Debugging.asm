PrintString:
    ;; Input
    ;; [ds:si] -> points to the null terminated string
    xor     ax, ax
    xor     bx, bx
    mov     al, 65
    int     10h
loop_printstr:  
    lodsb
    cmp     al, 0
    je      loop_printstr_end
    call    PrintChar
    jmp     loop_printstr
loop_printstr_end:  
    ret

PrintHex:
    ;; Inputs
    ;; dx -> Hex value to be printed
    mov     al, 030h            ;print '0'
    call    PrintChar
    mov     al, 078h            ;print 'x'
    call    PrintChar   
    mov     cx, 04h             ;counter
    rol     dx, 04h             ;shift 4 bits to left
loop_printhex:
    mov     ax, dx
    and     ax, 0Fh             ; al now has the correct hex value to be printed
    ;; convert to ASCII
    cmp     ax, 09h
    jg      convert_alpha       ;if number < 9 add 44 else add 55
    add     al, 48              ;ASCII 0 -9
    jmp     loop_print
convert_alpha:
    add     al, 037h            ;ASCII A-Z (65 - 10)
loop_print:
    call    PrintChar
    dec     cx
    jz      PrintHex_end
    rol     dx, 04h
    jmp     loop_printhex
PrintHex_end:
    ret

PrintEnter:
    mov     al, 0dh
    call    PrintChar
    mov     al, 0ah
    call    PrintChar
PrintEnter_end: 
    ret

PrintChar:
    ;; al -> will contain the char to be printed
    xor     bx, bx
    mov     ah, 0eh
    int     10h
    ret

PrintBinary:
    ;; dx -> binary to be printed
    mov     cx, 16
    rol     dx, 1
PrintBinary_loop:
    mov     ax, dx
    and     ax, 01h
    add     ax, 48
    call    PrintChar

    dec     cx
    jz      PrintBinary_end
    rol     dx, 1
    jmp     PrintBinary_loop
PrintBinary_end:
    ret
    
PrintFlags:
    pushf
    pop     dx
    call    PrintBinary
PrintFlags_end:
    ret
