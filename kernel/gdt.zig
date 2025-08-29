const serial = @import("serial.zig");
const hal = @import("hal.zig");
const NUM_SEG = 3;
const CODE_SEG = 0x8;
const DATA_SEG = 0x10;
const segment = @import("segment.zig").segment;

const GDT = struct {
    const GDTR = packed struct {
        limit: u16,
        base: u64,
        fn print(self: *GDTR) void {
            serial.write("GDTR {*}, base {*}: 0x{x} limit {*}: 0x{x}\n", .{
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
        //self.gdt[idx].print_size();
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
            : .{
              .memory = true,
              .rax = true,
              .rsp = true,
              .rbp = true,
            });
    }
};

var gdt: GDT = undefined;

pub fn Init() void {
    gdt = GDT.init();
    gdt.fill_table();
    gdt.fill_gdtr_struct();
    gdt.reload_segments();
}
