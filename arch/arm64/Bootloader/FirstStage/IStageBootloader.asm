    bits    16                      ; 16 bit code
    org     07c00h                  ; Jump after BIOS
    DEBUG   equ   0                 ; debuggin support
    UNREAL  equ   0

__start:
    ;; Entry point
    cli
    xor     ax, ax
    ;; Clear all the segment register, since we cannot garuntee what the BIOS has done
    mov     ds, ax
    mov     es, ax
    mov     ss, ax
    mov     sp, 07BFFh          ; set up the real mode stack
    jmp     boot

    ;; Includes
%if DEBUG
    %include "Debugging.asm"
%endif
    %include "Globals.inc"
    %include "A20.asm"
    %include "MMap.asm"

boot:
    mov     al, dl
    mov     [boot_drive], al    ; save our boot drive number
    call    EnableA20Gate
    call    GetMemMap
    jc      .end
    mov     dx, IISTAGE_SECTORS ; Read sectors
    call    Read2ndStageToMem
    call    SwitchToPMode
.end:
	;; ERROR: perofrm a warm boot. jump to reset vector
    jmp     0FFFFh:00h
    hlt
    ret

Read2ndStageToMem:
    ;; bx -> number of sectors to read
    pusha
    mov     cx, 02h             ; start from 2n sector
    mov     bx, IISTAGE_ADDRESS ; initial address
    xor     ax, ax
.loop:
    call    ReadSector
    inc     ax
    cmp     ax, dx
    je      .end
    add     bx, 0200h           ;point to next sector
    inc     cx                  ;read next sector
    jmp     .loop
.end:
    popa
    ret

ReadSector:
    ;; cx -> sector number to read
    ;; bx -> address
    pusha
    mov     dl, [boot_drive]
    mov     ax, 0201h
    int     13h
.end:
    popa
    ret

%if UNREAL
EnableBigSegments:
    ;; Switch to Unreal mode
    cli
    ;; Switch to protected mode
    lgdt    [GDTDescriptor]
    mov     eax, cr0
    or      al, 1
    mov     cr0, eax
    jmp     (CODE_SEGMENT << 3):.switch

    bits    32
.switch:
    ;; We are now in protected mode
    ;; save a segment
    xor     eax, eax
    mov     eax, (DATA_SEGMENT << 3)
    mov     es, eax
    mov     gs, eax
    mov     fs, eax

    ;; switch back to real mode
    mov     eax, cr0
    and     eax, 0x7FFFFFFe	; Disable paging bit & disable 16-bit pmode.
    mov     cr0, eax

    xor     eax, eax
    mov     ds, eax
    mov     cs, eax
    mov     ss, eax
    mov     es, eax
    jmp     .switchback
    bits    16
.switchback:
    xor     eax, eax
    mov     ds, eax
    mov     cs, eax
    mov     ss, eax
    mov     sp, 07BFFh          ; set up the real mode stack
    lgdt    [RealModeGDT]
.end:
    ret
%endif

SwitchToPMode:
    ;; Switch to protected mode
    cli
    lgdt    [GDTDescriptor]
    mov     eax, cr0
    or      al, 1
    mov     cr0, eax
    ;; jump to kernel
    push    MMAP_ADDRESS
    push    (DATA_SEGMENT << 3)
    jmp     (CODE_SEGMENT << 3):IISTAGE_ADDRESS ; since each entry is 8 bytes

.end:
	;; ERROR: perofrm a warm boot
    ;;  jump to reset vector
    jmp     0FFFFh:00h
    hlt
    hlt
    ret

%if DEBUG
PrintInitMessage:
    mov     si, WelcomeMessage
    call    PrintString
    call    PrintEnter
    ret

%endif
times 510 - ($-$$) db 0 ; pad remaining 510 bytes with zeroes
dw 0xaa55 ; magic bootloader magic - marks this 512 byte sector bootable!
