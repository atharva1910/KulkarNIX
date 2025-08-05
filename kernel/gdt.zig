const serial = @import("serial.zig");
const hal = @import("hal.zig");
const NUM_SEG = 3;
const CODE_SEG = 0x8;
const DATA_SEG = 0x10;

const segment = packed struct {
    limit_low: u16,
    base_low: u16,
    base_mid: u8,
    access: u8,
    limit_high: u4,
    flags: u4,
    base_last: u8,

    fn print_size(seg: *segment) void {
        serial.write("SEGMENT 0x{x}:{*}:\n\tlimit_low = 0x{x} -> {*} -> 0x{x}\n\tbase_low = 0x{x} -> {*} -> 0x{x}\n\tbase_mid = 0x{x} -> {*} -> 0x{x}\n\taccess = 0x{x} -> {*} -> 0x{x}\n\tlimit_high = 0x{x} -> {*} -> 0x{x}\n\tflags = 0x{x} -> {*} -> 0x{x}\n\tbase_last = 0x{x} -> {*} -> 0x{x}\n", .{
            @sizeOf(segment),                 seg,
            @sizeOf(@TypeOf(seg.limit_low)),  &seg.limit_low,
            seg.limit_low,                    @sizeOf(@TypeOf(seg.base_low)),
            &seg.base_low,                    seg.base_low,
            @sizeOf(@TypeOf(seg.base_mid)),   &seg.base_mid,
            seg.base_mid,                     @sizeOf(@TypeOf(seg.access)),
            &seg.access,                      seg.access,
            @sizeOf(@TypeOf(seg.limit_high)), &seg.limit_high,
            seg.limit_high,                   @sizeOf(@TypeOf(seg.flags)),
            &seg.flags,                       seg.flags,
            @sizeOf(@TypeOf(seg.base_last)),  &seg.base_last,
            seg.base_last,
        });
    }

    fn print_addr(seg: *segment) void {
        serial.write("SEGMENT:{*}:\n\tlimit_low = {*}\n\tbase_low = {*}\n\tbase_mid = {*}\n\taccess = {*}\n\tlimit_high = {*}\n\tflags = {*}\n\tbase_high = {*}\n\tbase_last = {*}\n", .{
            seg,
            &seg.limit_low,
            &seg.base_low,
            &seg.base_mid,
            &seg.access,
            &seg.limit_high,
            &seg.flags,
            &seg.base_last,
        });
    }

    fn print(seg: *segment) void {
        serial.write("SEGMENT:{*}:\n\tlimit_low = 0x{x}\n\tbase_low = 0x{x}\n\tbase_mid = 0x{x}\n\taccess = 0x{x}\n\tlimit_high = 0x{x}\n\tflags = 0x{x}\n\tbase_high = 0x{x}\n\tbase_last = 0x{x}\n", .{
            seg,
            seg.limit_low,
            seg.base_low,
            seg.base_mid,
            seg.access,
            seg.limit_high,
            seg.flags,
            seg.base_last,
        });
    }
};

const GDT = struct {
    const GDTR = packed struct {
        limit: u16,
        base: u64,
        fn print(self: *GDTR) void {
            serial.write("Loading GDTR {*}, base {*}: 0x{x} limit {*}: 0x{x}\n", .{
                self, &self.base, self.base, &self.limit, self.limit,
            });
        }
    };

    gdt: [NUM_SEG]segment,
    gdtr: GDTR,

    fn init() GDT {
        return .{
            .gdt = undefined,
            .gdtr = undefined,
        };
    }

    fn fill_gdtr_struct(self: *GDT) void {
        self.gdtr.base = @intFromPtr(&self.gdt[0]);
        self.gdtr.limit = @sizeOf(segment) * NUM_SEG - 1;
        self.gdtr.print();
    }

    fn fill_table(self: *GDT) void {
        self.init_gdt_entry(0, 0, 0, 0, 0);
        self.init_gdt_entry(1, 0, 0xFFFFF, 0x9A, 0xA);
        self.init_gdt_entry(2, 0, 0xFFFFF, 0x92, 0xA);
        self.gdtr.base = @intFromPtr(&self.gdt[0]);
        self.gdtr.limit = @sizeOf(segment) * NUM_SEG - 1;
    }

    fn init_gdt_entry(self: *GDT, idx: usize, base: u64, limit: u20, access: u8, flags: u4) void {
        self.gdt[idx].limit_low = @truncate(limit);
        self.gdt[idx].base_low = @truncate(base);
        self.gdt[idx].base_mid = @truncate(base >> 16);
        self.gdt[idx].access = access;
        self.gdt[idx].limit_high = @truncate(limit >> 16);
        self.gdt[idx].flags = flags;
        self.gdt[idx].base_last = @truncate(base >> 32);
        self.gdt[idx].print_size();
    }

    fn reload_segments(self: *GDT) void {
        asm volatile (
            \\xor %%rax, %%rax
            \\lea flush(%%rip), %%rax
            \\push %[cs]            
            \\push %%rax
            \\lgdt %[gdtr]
            \\lretq
            \\flush:
            \\movw %[ds], %%ax
            \\movw %%ax, %%ds
            \\movw %%ax, %%ss
            \\movw %%ax, %%es
            \\movw %%ax, %%fs
            \\movw %%ax, %%gs
            :
            : [gdtr] "*p" (&self.gdtr),
              [cs] "i" (CODE_SEG),
              [ds] "i" (DATA_SEG),
            : "memory", "rax", "rsp", "rbp"
        );
    }
};

var gdt: GDT = undefined;

pub fn init() void {
    gdt = GDT.init();
    gdt.fill_table();
    gdt.fill_gdtr_struct();
    gdt.reload_segments();
}
