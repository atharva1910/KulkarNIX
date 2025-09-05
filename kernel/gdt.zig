const Serial = @import("serial.zig");
const hal = @import("hal.zig");
const NUM_SEG = 3;
const CODE_SEG = 0x8;
const DATA_SEG = 0x10;
const segment = @import("segment.zig").segment;
const KError = @import("kerrors.zig");

var gdt: GDT = undefined;
var gdtr: GDTR = undefined;

const GDTR = packed struct {
    limit: u16,
    base: u64,

    fn Print(self: *GDTR) void {
        Serial.Write("GDTR {*}, base {*}: 0x{x} limit {*}: 0x{x}\n", .{
            self, &self.base, self.base, &self.limit, self.limit,
        });
    }

    fn Init(self: *GDTR, base: usize) void {
        self.base = base;
        self.limit = @sizeOf(segment) * NUM_SEG - 1;
    }
};

const GDT = struct {
    gdt: [NUM_SEG]segment,

    fn Init(self: *GDT) usize {
        self.InitEntry(0, 0, 0, 0, 0);
        self.InitEntry(1, 0, 0xFFFFF, 0x9A, 0xA);
        self.InitEntry(2, 0, 0xFFFFF, 0x92, 0xA);
        return @intFromPtr(&self.gdt[0]);
    }

    fn InitEntry(self: *GDT, idx: usize, base: u64, limit: u20, access: u8, flags: u4) void {
        self.gdt[idx].LimitLow = @truncate(limit);
        self.gdt[idx].BaseLow = @truncate(base);
        self.gdt[idx].BaseMid = @truncate(base >> 16);
        self.gdt[idx].Access = access;
        self.gdt[idx].LimitHigh = @truncate(limit >> 16);
        self.gdt[idx].Flags = flags;
        self.gdt[idx].BaseLast = @truncate(base >> 32);
    }
};

pub fn Init() void {
    const base = gdt.Init();
    gdtr.Init(base);

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
        : [gdtr] "*p" (&gdtr),
          [cs] "i" (CODE_SEG),
          [ds] "i" (DATA_SEG),
        : .{
          .memory = true,
          .rax = true,
          .rsp = true,
          .rbp = true,
        });

    gdtr.Print();
}
