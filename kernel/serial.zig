const COM1 = 0x3F8;
const bufPrint = @import("std").fmt.bufPrint;
const hal = @import("hal.zig");

pub fn init() bool {
    hal.outb(COM1 + 1, 0x0);
    hal.outb(COM1 + 3, 0x80);
    hal.outb(COM1, 0x1);
    hal.outb(COM1 + 1, 0x0);
    hal.outb(COM1 + 3, 0x03);
    hal.outb(COM1 + 2, 0xC7);
    hal.outb(COM1 + 4, 0x08);
    hal.outb(COM1 + 4, 0x1E);
    hal.outb(COM1, 0xFF);

    if (hal.inb(COM1) != 0xFF) {
        return false;
    }

    hal.outb(COM1 + 4, 0x0F);
    return true;
}

pub fn Write(comptime str: []const u8, args: anytype) void {
    var u8buf = [_]u8{0} ** 512;
    _ = bufPrint(u8buf[0..], str, args) catch {
        return;
    };

    for (u8buf) |c| {
        hal.outb(COM1, c);
    }
}

pub fn writestr(comptime str: []const u8) void {
    for (str) |c| {
        hal.outb(COM1, c);
    }
}
