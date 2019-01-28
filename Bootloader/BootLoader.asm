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
    %include "IO.asm"

boot:
    mov     al, dl              ;
    mov     [boot_drive], al    ; save our boot drive number
    ;; Test Print String
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
    mov     cl, 02h           ; read the 2nd sector
    mov     bx, 08000h        ; Address to load
    call    ReadSector
    jnc     .end
    mov     si, ErrorString
    call    PrintString
    hlt
.end:
    popa
    ret

ErrorString:    db "ERROR ERROR ERRROR", 0

    
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
    ;; offset into the GDTs code segment. 
    jmp     (CODE_SEGMENT << 3):ProtectedModeEntry ; since each entry is 8 bytes
.end:
	;; ERROR: perofrm a warm boot
    ;;  jump to reset vector
    jmp     0FFFFh:00h
    hlt
    hlt
    ret

myGDT:  
    NULLGDT_ENTRY                                   ;null entry
    GDT_ENTRY 0 ,0FFFFFFFFh ,(GDT_EXECUTE|GDT_READ) ;code segment
    GDT_ENTRY 0 ,0FFFFFFFFh ,GDT_WRITE              ;data segment

GDTDescriptor:
    dw (GDTDescriptor - myGDT - 1) ;size
    dd myGDT                       ;offset to GDT
   
    ;; WE are now in protected mode :)
    bits    32
ProtectedModeEntry:
    mov     ax, (DATA_SEGMENT << 3)
    mov     ds, ax
    mov     ss, ax
    mov     es, ax
    ;; jump to kernel
    ;; jump to extended BIOS memory
    push    08000h
    jmp     (CODE_SEGMENT << 3):08000h
    hlt
.end:
    ret

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


times 510 - ($-$$) db 0 ; pad remaining 510 bytes with zeroes
dw 0xaa55 ; magic bootloader magic - marks this 512 byte sector bootable!
