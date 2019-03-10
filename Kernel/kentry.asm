; Declare constants for the multiboot header.
MBALIGN  equ  1 << 0            ; align loaded modules on page boundaries
MEMINFO  equ  1 << 1            ; provide memory map
FLAGS    equ  MBALIGN | MEMINFO ; this is the Multiboot 'flag' field
MAGIC    equ  0x1BADB002        ; 'magic number' lets bootloader find the header
CHECKSUM equ -(MAGIC + FLAGS)   ; checksum of above, to prove we are multiboot
 
; Declare a multiboot header that marks the program as a kernel. These are magic
; values that are documented in the multiboot standard. The bootloader will
; search for this signature in the first 8 KiB of the kernel file, aligned at a
; 32-bit boundary. The signature is in its own section so the header can be
; forced to be within the first 8 KiB of the kernel file.
section .multiboot
align 4
	dd MAGIC
	dd FLAGS
	dd CHECKSUM

section .text
    bits 32
    global __start
    extern kernel_main
__start:
    ;; We have jumped here from the bootloader
    ;; Set back the segment registers, set up the stack
    ;; call C++ code
    ;xor     eax, eax
    ;mov     ds, eax
    ;mov     ss, eax
    ;mov     es, eax
    mov     sp, 07c00h
    cli
    call    kernel_main
    hlt
    hlt

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

    %if 0
section .data
WelcomeString: db "Welcome to the Kernel",0
TestSting: db "After read",0
    %endif

