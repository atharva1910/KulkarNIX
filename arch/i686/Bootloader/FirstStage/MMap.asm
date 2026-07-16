GetMemMap:
    ;; Gets the memory map and stores it at MMAP_ADDRESS
    ;; will be passed as a parameter to the kernel
    pusha
    xor ebx, ebx                ;the first call
    xor edx, edx                
    
    xor esi, esi
    mov edi, MMAP_ADDRESS       ;dest address
    add edi, 16                 ;First 16bytes will store the counter
.loop:
    xor eax, eax
    mov eax, 0E820h             ;function
    mov edx, 0534D4150h         ;SMAP
    mov ecx, 24                 ;request 24 bytes
    int 15h                     ;call interrupt 15

    jc  .failed
    test ebx, ebx               ;exit condition, ebx = 0
    je  .end

    test cx, cx                 ;Empty entry
    je .loop
    
    cmp cx, 24                  ;ACPI comp
    je .next
    mov [edi + 20], dword 1     ;Dummy ACPI
.next:
    add di, 24                  ;point to next location
    mov eax, [MMAP_ADDRESS]
    inc eax
    mov [MMAP_ADDRESS], eax
    jmp .loop
.failed:
    hlt
    stc
.end:
    popa
    ret
    
