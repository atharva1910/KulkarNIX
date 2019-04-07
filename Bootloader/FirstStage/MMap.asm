GetMemMap:
    ;; Gets the memory map and stores it at MMAP_ADDRESS
    ;; will be passed as a parameter to the kernel
    pusha
    xor ebx, ebx                ;the first call
    mov edi, 050h             ;dest address
.loop:
    xor eax, eax
    mov eax, 0E820h             ;fun
    mov edx, 0534D4150h         ;SMAP
    mov ecx, 24                 ;request 24 bytes
    int 15h                     ;call interrupt 15

    jc  .failed                 
    cmp eax, edx                ;eax is set to SMAP after interrupt
    jne .failed
    test ebx, ebx               ;exit condition, ebx = 0
    je  .end

    add di, 24                  ;point to next location
    jmp .loop
.failed:
    ;; Print something?
.end:
    popa
    ret

mmap_count: dd 0

    %if 0
PrintHex:
    ;; Inputs
    ;; dx -> Hex value to be printed
    pusha
    mov     al, 030h            ;print '0'
    call    PrintChar
    mov     al, 078h            ;print 'x'
    call    PrintChar   
    mov     cx, 04h             ;counter
    rol     dx, 04h             ;shift 4 bits to left
.loop_printhex:
    mov     ax, dx
    and     ax, 0Fh             ; al now has the correct hex value to be printed
    ;; convert to ASCII
    cmp     ax, 09h
    jg      .convert_alpha       ;if number < 9 add 44 else add 55
    add     al, 48              ;ASCII 0 -9
    jmp     .loop_print
.convert_alpha:
    add     al, 037h            ;ASCII A-Z (65 - 10)
.loop_print:
    call    PrintChar
    dec     cx
    jz      .end
    rol     dx, 04h
    jmp     .loop_printhex
.end:
    popa
    ret

PrintEnter:
    mov     al, 0dh
    call    PrintChar
    mov     al, 0ah
    call    PrintChar
.end:
    ret

PrintChar:
    ;; al -> will contain the char to be printed
    xor     bx, bx
    mov     ah, 0eh
    int     10h
    ret

    %endif
