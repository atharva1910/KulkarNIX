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

