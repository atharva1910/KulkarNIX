const Serial = @import("serial.zig");
const kargs = @import("kargs.zig").kargs;
const GDT = @import("gdt.zig");
const KError = @import("kerrors.zig").KError;

const PMem = @import("pmem.zig");
const PMemManager = PMem.PMemManager;
const Heap = @import("heap.zig");
const HeapManager = Heap.HeapManager;
const Paging = @import("paging.zig");
const PageTableMgr = @import("paging.zig").PageTableMgr;

const HAL = @import("hal.zig");
const KState = @import("kstate.zig").KState;

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

    Serial.Write("Welcome to the kernel. Args; {*} Kernel PAddr: 0x{x}\n", .{
        args,
        args.?.KernelPAddr,
    });

    GDT.Init();

    PMem.Init(
        args.?.KMemMap,
        args.?.PageTableManger,
        args.?.KernelPAddr + args.?.KDataOffset,
        args.?.KCodePages,
    ) catch |err| {
        Serial.Write("Failed to initialize PMEM status: {}\n", .{err});
    };

    Paging.Init(args.?.PageTableManger);

    Heap.Init() catch |err| {
        Serial.Write("Failed to initialize Heap status: {}\n", .{err});
    };

    HAL.hlt();
}
