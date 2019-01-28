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
    mov     sp, 08000h          ; set up the real mode stack
    jmp     boot

    ;; Includes
    %include "Debugging.asm"
    %include "Globals.asm"
    %include "A20.asm"

boot:
    mov     al, dl              ;
    mov     [boot_drive], al    ; save our boot drive number

    ;; Test Print String
    call    PrintInitMessage
    call    IsA20GateEnabled
    jc      .skipA20enable
    call    EnableA20Gate       
.skipA20enable:
    call    SwitchToPMode
.end:
	;; ERROR: perofrm a warm boot
    ;;  jump to reset vector
    jmp     0FFFFh:00h
    hlt
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
    ;; 08 -> offset into the GDTs code segment. 
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
    ;; Test to see if we really are in protected mode
    mov     eax, cr0
    and     eax, 1
    cmp     eax, 1
    je      .runningInProtectedMode
    xor     bx, bx
    mov     ah, 0Ah
    int     10h
    jmp     .end
.runningInProtectedMode:
    ;; Yes
    ;; Print Hi on the top left, CGA mode
    mov     ax, (DATA_SEGMENT << 3)
    mov     ds, ax
    mov     ss, ax
    mov     es, ax
    call    PPrintString
    ;; jump to kernel
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
