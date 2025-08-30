const serial = @import("serial.zig");
const kargs = @import("kargs.zig").kargs;
const GDT = @import("gdt.zig");
const KError = @import("kerrors.zig");
const PMem = @import("pmem.zig");

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
        return;
    }

    serial.write("Welcome to the kernel. Kernel args {*} 0x{x}\n", .{ args, args.?.KPAddr });

    GDT.Init();
    //
    PMem.Init(
        args.?.KMemMap,
        args.?.KMemPages,
    ) catch |err| {
        serial.write("Failed to initialize PMEM status: {}\n", .{err});
    };
}
