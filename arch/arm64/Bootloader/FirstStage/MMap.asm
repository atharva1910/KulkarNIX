GetMemMap:
    ;; Gets the memory map and stores it at MMAP_ADDRESS
    ;; will be passed as a parameter to the kernel
    pusha
    xor ebx, ebx                ;the first call
    mov edi, MMAP_ADDRESS       ;dest address
.loop:
    xor eax, eax
    mov eax, 0E820h             ;function
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
    stc
.end:
    popa
    ret

mmap_count: dd 0
