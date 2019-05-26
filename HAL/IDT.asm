    .global asm_load_idt
asm_load_idt:
    # This loads the empty IDT 
    pusha
    lidt (__idtr)
    popa
    ret
    
