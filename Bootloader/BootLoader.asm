    bits    16                      ; 16 bit code
    org     07c00h                  ; Jump after BIOS

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
    %include "Debugging.asm"
    %include "Globals.asm"
    %include "A20.asm"

boot:
    mov     al, dl              ;
    mov     [boot_drive], al    ; save our boot drive number
    call    PrintInitMessage
    call    IsA20GateEnabled
    jc      .skipA20enable
    call    EnableA20Gate       
.skipA20enable:
    call    ReadSecondSectorToMemory
    call    SwitchToPMode
.end:
	;; ERROR: perofrm a warm boot. jump to reset vector
    jmp     0FFFFh:00h
    hlt
    ret

ReadSecondSectorToMemory:
    pusha
    xor     cx, cx
    xor     dx, dx
    mov     dl, [boot_drive]
    mov     cl, 02h                   ; read the 2nd sector
    mov     bx, KERNEL_ADDRESS        ; Address to load
    mov     ax, 0201h
    int     13h
    jnc     .end
    mov     si, KernelReadFailStr
    call    PrintString
    hlt
.end:
    popa
    ret

PrintInitMessage:
    mov     si, WelcomeMessage
    call    PrintString
    call    PrintEnter
    ret

SwitchToPMode:
    ;; Switch to protected mode
    cli
    lgdt    [GDTDescriptor]
    mov     eax, cr0
    or      al, 1
    mov     cr0, eax
    ;; jump to kernel
    push    (DATA_SEGMENT << 3)
    jmp     (CODE_SEGMENT << 3):KERNEL_ADDRESS ; since each entry is 8 bytes
     
.end:
	;; ERROR: perofrm a warm boot
    ;;  jump to reset vector
    jmp     0FFFFh:00h
    hlt
    hlt
    ret
  
times 510 - ($-$$) db 0 ; pad remaining 510 bytes with zeroes
dw 0xaa55 ; magic bootloader magic - marks this 512 byte sector bootable!
