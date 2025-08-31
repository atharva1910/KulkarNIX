const Serial = @import("serial.zig");
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

    Serial.Write("Welcome to the kernel. Kernel args {*} 0x{x}\n", .{
        args,
        args.?.KernelPAddr,
    });

    Serial.Write("PageTables: {*} Len: 0x{x}\n", .{
        args.?.PageTables.ptr,
        args.?.PageTables.len,
    });

    GDT.Init();

    _ = PMem.Init(
        args.?.KMemMap,
        args.?.KDataPages,
        args.?.KernelPAddr + args.?.KCodeOffset,
        args.?.KCodePages,
    ) catch |err| {
        Serial.Write("Failed to initialize PMEM status: {}\n", .{err});
    };
}
