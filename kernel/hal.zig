pub fn outb(comptime p: u16, c: u8) void {
    asm volatile (
        \\ outb %%al, %[p]
        :
        : [c] "{al}" (c),
          [p] "{dx}" (p),
    );
}

pub fn inb(comptime p: u16) u8 {
    return asm volatile (
        \\ inb %[p], %[ret]
        : [ret] "={al}" (-> u8),
        : [p] "{dx}" (p),
    );
}

pub fn hlt() void {
    asm volatile (
        \\hlt
    );
}

pub fn lgdt(pgdt: u64) void {
    asm volatile (
        \\lgdt (%%r13)
        :
        : [gdtr] "r13" (pgdt),
    );
}

pub fn sgdt() u64 {
    return asm volatile (
        \\sgdt (%[ret])
        : [ret] "={rax}" (-> u64),
    );
}

pub fn cli() void {
    asm volatile (
        \\cli
    );    
}
