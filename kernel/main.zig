const serial = @import("serial.zig");
const PMem = @import("pmem.zig");
const hal = @import("hal.zig");
const kargs = @import("kargs.zig").kargs;
const GDT = @import("gdt.zig");

export var stack_bytes: [16 * 1024]u8 = undefined;

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
        \\call *%rax
        \\hlt
    );
}

export fn kmain() void {
    var args: ?*kargs = undefined;
    asm volatile (
        \\movq %%r13, %[kargs]
        : [kargs] "={r13}" (args),
    );

    if (args == null) {
        serial.write("No Arguments\n", .{});
        return;
    }

    serial.write("Welcome to the kernel. Kernel args {*}. Kernel paddr 0x{x} Kernel vaddr 0x{x} Kernel size 0x{x}\n", .{
        args,
        args.?.KPAddr,
        args.?.KOffset,
        args.?.KSize,
    });

    GDT.init();
    PMem.Init(
        args.?.KMemMap,
        args.?.KMemMapSize,
        args.?.DescSize,
        args.?.KMemPages,
    );
}
