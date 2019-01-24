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

boot:
    ;; Test Print String
    call    PrintInitMessage
    call    IsA20GateEnabled
    jc      .skipA20enable
    call    EnableA20Gate       
.skipA20enable:
    ;call    LoadGDT
    call    SwitchToPMode
.end:
    hlt
    ret
    
PrintInitMessage:
    mov     si, WelcomeMessage
    call    PrintString
    call    PrintEnter
    ret

IsA20GateEnabled:
    ;; compare the boot signature (0000:7DFE) with (FFFF:7DFE)
    ;; Output -> set carry flag if a20 gate is enabled

    ;; Read from memory into ax and bx
    push    ds                  
    xor     ax, ax
    not     ax                  ; 0FFFFh

    mov     ds, ax
    mov     si, 07DFEh
    mov     ax, word [ds:si]

    pop     ds
    cmp     word [ds:si], ax
    je      .end
    stc                         ; a20 gate is enabled
    ;; Todo todo todo
    ;; This could be pure luck, change the signaature and test again!!
.end:
    ret

    ;; Enable 21st bit addressing
    ;; NOTE NOTE NOTE: not tested
EnableA20Gate:
    xor     ax, ax
    ;; Make sure 8042 is ready for inputs
    call    FlushPS2InputBuffers
    ;; Talk to PS2 controller to read the current chip config
    mov     al, 0D0h
    out     064h, al
    ;; wait for output
    call    FlushPS2OutputBuffers
    ;; read config
    in      al, 060h
    ;; enable 2nd bit for a20 support
    or      al, 02h
    ;; send the command back 
    call    FlushPS2InputBuffers
    out     060h, al
    call    FlushPS2OutputBuffers
.end:
    ret

FlushPS2OutputBuffers:
    ;; Output buffer from the point of 8042 chip
    ;; This function waits for the output buffer to be set
    pusha
.loop:
    xor     ax, ax
    in      al, 064h
    test    al, 1               ; 1st bit is set if buffer is full
    jz      .loop ; wait till output is set
.end:
    popa
    ret

FlushPS2InputBuffers:
    ;; Input buffer from the point of 8042 chip
    pusha
.loop:
    xor     ax, ax
    in      al, 064h
    test    al, 2               ; 2nd bit is set if buffer is full
    jnz     .loop ; wait till input buffer is cleared
.end:   
    popa
    ret

SwitchToPMode:
    ;; Switch to protected mode
    cli
    lgdt    [GDTDescriptor]
    mov     eax, cr0
    or      al, 1
    mov     cr0, eax
    ;; 08 -> offset into the GDT. 
    jmp     08h:ProtectedModeEntry
.end:
    ;; Probaly never get here
    hlt
    hlt
    hlt
    ret

myGDT:  
;    NULLGDT_ENTRY                                   ;null entry
;    GDT_ENTRY 0 ,0FFFFFFFFh ,(GDT_EXECUTE|GDT_READ) ;code segment
;    GDT_ENTRY 0 ,0FFFFFFFFh ,GDT_WRITE              ;data segment

    ;; Null
  dw 0000h
  db 00h
  db 00000000b
  db 00000000b
  db 00h

    ;; Code
  dw 0ffffh
  dw 0000h
  db 00h
  db 10011010b
  db 11001111b
  db 00h

    ;; Data
  dw 0ffffh
  dw 0000h
  db 0x00
  db 10010010b
  db 11001111b                 ;
  db 00h

GDTDescriptor:
    dw (GDTDescriptor - myGDT - 1) ;size
    dd myGDT                       ;offset to GDT
   
    bits    32
ProtectedModeEntry:
    hlt
.end:
    ret
   
ProtectedModeWelcomeString: db "Welcome to protected mode"
times 510 - ($-$$) db 0 ; pad remaining 510 bytes with zeroes
dw 0xaa55 ; magic bootloader magic - marks this 512 byte sector bootable!
