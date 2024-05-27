GetMemMap:
    ;; Gets the memory map and stores it at MMAP_ADDRESS
    ;; will be passed as a parameter to the kernel
    pusha
    xor ebx, ebx                ;the first call
    xor esi, esi
    mov edi, MMAP_ADDRESS       ;dest address
.loop:
    xor eax, eax
    mov eax, 0E820h             ;function
    mov edx, 0534D4150h         ;SMAP
    mov ecx, 24                 ;request 24 bytes
    int 15h                     ;call interrupt 15

    jc  .failed
    test ebx, ebx               ;exit condition, ebx = 0
    je  .end

    cmp cx, 24
    je .next
    mov [edi + 20], dword 1
.next:
    add di, 24                  ;point to next location
    jmp .loop
.failed:
    hlt
    stc
.end:
    popa
    ret
