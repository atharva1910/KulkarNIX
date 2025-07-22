const COM1 = 0x3F8;
const bufPrint = @import("std").fmt.bufPrint;

fn outb(comptime p: u16, c: u8) void {
    asm volatile (
        \\ outb %%al, %[p]
        :
        : [c] "{al}" (c),
          [p] "{dx}" (p),
    );
}

fn inb(comptime p: u16) u8 {
    return asm volatile (
        \\ inb %[p], %[ret]
        : [ret] "={al}" (-> u8),
        : [p] "{dx}" (p),
    );
}

pub fn init() bool {
    outb(COM1 + 1, 0x0);
    outb(COM1 + 3, 0x80);
    outb(COM1, 0x1);
    outb(COM1 + 1, 0x0);
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x08);
    outb(COM1 + 4, 0x1E);
    outb(COM1, 0xFF);

    if (inb(COM1) != 0xFF) {
        return false;
    }

    outb(COM1 + 4, 0x0F);
    return true;
}

pub fn write(comptime str: []const u8, args: anytype) void {
    var u8buf: [256]u8 = [_]u8{0} ** 256;
    _ = bufPrint(u8buf[0..], str, args) catch {
        return;
    };

    for (u8buf) |c| {
        outb(COM1, c);
    }
}
