const serial = @import("serial.zig");
const hal = @import("hal.zig");
const NUM_SEG = 3;
const CODE_SEG = 0x8;
const DATA_SEG = 0x10;

const GDTR = packed struct {
    limit: u16,
    base: u64,
};

const segment = packed struct {
    limit_low: u16,
    base_low: u16,
    base_mid: u8,
    access: u8,
    limit_high: u4,
    flags: u4,
    base_high: u8,
    base_last: u32,

    fn print_segment(seg: *segment) void {
        serial.write("SEGMENT:{*}:\n\tlimit_low = 0x{x}\n\tbase_low = 0x{x}\n\tbase_mid = 0x{x}\n\taccess = 0x{x}\n\tlimit_high = 0x{x}\n\tflags = 0x{x}\n\tbase_high = 0x{x}\n\tbase_last = 0x{x}\n", .{
            seg,
            seg.limit_low,
            seg.base_low,
            seg.base_mid,
            seg.access,
            seg.limit_high,
            seg.flags,
            seg.base_high,
            seg.base_last,
        });
    }
};

const GDT = struct {
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
        serial.write("Loading GDTR {*}, base {*}: 0x{x} limit {*}: 0x{x}\n", .{
            &self.gdtr,
            &self.gdtr.base,
            self.gdtr.base,
            &self.gdtr.limit,
            self.gdtr.limit,
        });
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
        self.gdt[idx].limit_high = @truncate(limit >> 16);
        self.gdt[idx].base_high = @truncate(base >> 24);
        self.gdt[idx].base_last = @truncate(base >> 32);
        self.gdt[idx].flags = flags;
        self.gdt[idx].access = access;
        self.gdt[idx].print_segment();
    }
};

fn reload_segments(self: *GDT) void {
    asm volatile (
        \\lgdt %[gdtr]
        \\push %[cs]
        \\push %[dummy]
        \\lretq
        :
        : [gdtr] "*p" (&self.gdtr),
          [cs] "i" (CODE_SEG),
          [dummy] "r" (&reload_ds),
        : "stack"
    );
}

fn reload_ds() void {
    asm volatile (
        \\xor %%rax, %%rax
        \\movw %[ds], %%ax
        \\movw %%ax, %%ds
        \\movw %%ax, %%ss
        \\movw %%ax, %%es
        \\movw %%ax, %%fs
        \\movw %%ax, %%gs
        :
        : [ds] "i" (DATA_SEG),
        : "rax"
    );
}

pub fn init() void {
    var gdt = GDT.init();
    gdt.fill_table();
    gdt.fill_gdtr_struct();
    reload_segments(&gdt);
    //gdt[0].print_segment();
    //gdt[1].print_segment();
    //gdt[2].print_segment();

}
