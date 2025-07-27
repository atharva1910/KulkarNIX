const serial = @import("serial.zig");
const hal = @import("hal.zig");
const num_seg = 3;

const segment = packed struct {
    limit_low: u16,
    base_low: u16,
    base_mid: u8,
    access: u8,
    limit_high: u4,
    flags: u4,
    base_high: u8,
    base_last: u32,
};

const gdt_values = packed struct {
    limit: u16,
    base: u64,
};

var gdt: [num_seg]segment = [_]segment{.{
    .limit_low = 0,
    .base_low = 0,
    .base_mid = 0,
    .access = 0,
    .limit_high = 0,
    .flags = 0,
    .base_high = 0,
    .base_last = 0,
}} ** num_seg;
var gdtr: gdt_values = undefined;

fn init_gdt_entry(idx: usize, base: u64, limit: u20, access: u8, flags: u4) void {
    gdt[idx].limit_low = @truncate(limit);
    gdt[idx].base_low = @truncate(base);
    gdt[idx].base_mid = @truncate(base >> 16);
    gdt[idx].limit_high = @truncate(limit >> 16);
    gdt[idx].base_high = @truncate(base >> 24);
    gdt[idx].base_last = @truncate(base >> 32);
    gdt[idx].flags = flags;
    gdt[idx].access = access;
}

fn init_gdtr() void {
    gdtr.base = @intFromPtr(&gdt[0]);
    gdtr.limit = @sizeOf(segment) * num_seg - 1;
}

pub fn init() void {
    init_gdt_entry(1, 0, 0xFFFFF, 0x9A, 0xA);
    init_gdt_entry(2, 0, 0xFFFFF, 0x92, 0xA);
    init_gdtr();
    serial.write("Loading GDTR {*}, base {*} limit {*}\n", .{ &gdtr, &gdtr.base, &gdtr.limit });
    hal.lgdt(@intFromPtr(&gdtr));
    serial.write("Reading GDTR 0x{x}\n", .{hal.sgdt()});
}
