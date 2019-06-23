    .global asm_load_idt
asm_load_idt:
    # This loads the empty IDT 
    pusha
    lidt (_idtr)
    popa
    ret

    .global DefaultIDTfun
DefaultIDTfun:
    # This is the default IDT function for testing
    pusha
    call    DefaultFunction
    iret

    .global Interrupt000
Interrupt000:
    pusha
    call    KInterrupt000
    iret

    .global Interrupt001
Interrupt001:
    pusha
    call    KInterrupt001
    iret

     .global Interrupt002
Interrupt002:
    pusha
    call    KInterrupt002
    iret

    .global Interrupt003
Interrupt003:
    pusha
    call    KInterrupt003
    iret

    .global Interrupt004
Interrupt004:
    pusha
    call    KInterrupt004
    iret

    .global Interrupt005
Interrupt005:
    pusha
    call    KInterrupt005
    iret

    .global Interrupt006
Interrupt006:
    pusha
    call    KInterrupt006
    iret

    .global Interrupt007
Interrupt007:
    pusha
    call    KInterrupt007
    iret

    .global Interrupt008
Interrupt008:
    pusha
    call    KInterrupt008
    iret
