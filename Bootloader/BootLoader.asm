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
    jmp     boot

    ;; Includes
    %include "Debugging.asm"
    %include "Globals.asm"
    %include "A20.asm"

boot:
    ;; Test Print String
    call    PrintInitMessage
    call    IsA20GateEnabled
    jc      .skipA20enable
    call    EnableA20Gate       
.skipA20enable:
    call    SwitchToPMode
.end:
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
    ;; Probaly never get here
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
.runningInProtectedMode:
    ;; Yes
    ;; Print Hi on the top left, CGA mode
    mov     dword [0b8000h], 007690748h ;
    hlt
    hlt
    mov     ax, (DATA_SEGMENT << 3)
    mov     ds, ax
    mov     ss, ax
    mov     es, ax

    xor     bx, bx
    mov     ah, 0Ah
    int     10h
    hlt
.end:
    ret
ProtectedModeWelcomeString: db "Welcome to protected mode"

times 510 - ($-$$) db 0 ; pad remaining 510 bytes with zeroes
dw 0xaa55 ; magic bootloader magic - marks this 512 byte sector bootable!
