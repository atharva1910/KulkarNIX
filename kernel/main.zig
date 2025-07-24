const serial = @import("serial.zig");
const pmem = @import("pmem.zig");
const hal = @import("hal.zig");
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
        \\call *%rax
        \\hlt
    );
}

export fn kmain() void {
    var mmap: ?[*]align(4096) u8 = null;
    var desc_size: usize = undefined;
    var mmap_size: usize = undefined;
    asm volatile (
        \\movq %%r13, %[mmap]
        \\movq %%r14, %[mmap_size]            
        \\movq %%r15, %[desc_size]
        : [mmap] "={r13}" (mmap),
          [desc_size] "={r15}" (desc_size),
          [mmap_size] "={r14}" (mmap_size),
    );

    if (mmap == null) {
        serial.write("NULL MEMORY MAP\n", .{});
        hal.hlt();
    }

    serial.write("Welcome to the kernel", .{});
    pmem.init(mmap, mmap_size, desc_size);
}
