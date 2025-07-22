const serial = @import("serial.zig");
export var stack_bytes: [1024]u64 = undefined;

comptime {
    asm (
        \\.extern kmain
        \\.extern stack_bytes
        \\.global  _start
        \\.type _start, @function
        \\_start:
        \\movabs $stack_bytes, %rax
        \\addq $10000, %rax
        \\movq %rax, %rsp
        \\movq %rsp, %rbp
        \\movabs $kmain, %rax
        \\jmpq *%rax
    );
}

export fn kmain() void {
    serial.writestr("Test");
    asm volatile (
        \\hlt
    );
}
