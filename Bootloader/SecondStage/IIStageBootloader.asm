    bits 32
    section .text
    global __start
    extern boot_main
    extern read_sector
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
    mov     si, WelcomeString
    call    PPrintString
    hlt
    hlt

    %if 0
    mov     edx, 1
    call    ata_disk_wait

    mov     si, WelcomeString
    call    PPrintString

    call    ReadSector

    call    ata_disk_wait

    mov     si, TestSting
    call    PPrintString

    mov     dx, 01F0h
    mov     edi, 010000h
    mov     ecx, 0200h
    rep     insb
    hlt

    hlt
    hlt
    hlt

    global ata_bsy_wait
ata_bsy_wait:
    pusha
    xor al, al
    mov dx, 01F7h
.loop:   
    in  al, dx
    test al, 080h
    jnz .loop
.end:
    popa
    ret

ReadSector: 
    pusha
    ;; edx -> sector number
    mov ebx, edx
    
    ;; Read one sector
    mov dx,01F2h
    mov al, 1
    out dx, al

    ;; low 8 bits of LBA
    mov dx, 01F3h
    mov al, bl
    out dx, al

    ;; next 8 bits of LBA
    mov dx, 01F4h
    shr ebx, 8
    mov al, bl
    out dx, al

    ;; next 8 bits of LBA
    mov dx, 01F5h
    shr ebx, 8
    mov al, bl
    out dx, al

    ;; next 8 bits of LBA
    mov dx, 01F6h
    shr ebx, 8
    mov al, bl
    ;; last 4 bits of LBA
    and al, 0Fh
    ;; select master drive
    or al, 0E0h
    out dx, al

    ;; Read sectors
    mov dx, 01F7h
    mov al, 020h
    out dx, al
    popa
    ret
    %endif

    global ata_disk_wait
ata_disk_wait:
    pusha
    xor ax, ax
    mov dx, 01F7h
.loop:
    in  al, dx
    and al, 0C0h
    cmp al, 040h
    jne .loop
.end:
    popa
    ret

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

