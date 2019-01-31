    bits    16                      ; 16 bit code
    org     07c00h                  ; Jump after BIOS
    DEBUG   equ   0                  ; debuggin support

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
    %include "Globals.asm"
    %include "A20.asm"

boot:
    mov     al, dl              
    mov     [boot_drive], al    ; save our boot drive number
    call    IsA20GateEnabled
    jc      .skipA20enable
    call    EnableA20Gate       
.skipA20enable:
;    call    EnableBigSegments
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
%if DEBUG
    mov     si, KernelReadFailStr
    call    PrintString
%else
    jmp $
%endif
    hlt
.end:
    popa
    ret

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

%if DEBUG
PrintInitMessage:
    mov     si, WelcomeMessage
    call    PrintString
    call    PrintEnter
    ret

%endif
times 510 - ($-$$) db 0 ; pad remaining 510 bytes with zeroes
dw 0xaa55 ; magic bootloader magic - marks this 512 byte sector bootable!
