const uefi = @import("std").os.uefi;
const status = uefi.Status;
const L = @import("std").unicode.utf8ToUtf16LeStringLiteral;
const bufPrint = @import("std").fmt.bufPrint;

pub fn init() void {
    return;
}

pub fn printString(comptime str: []const u8, args: anytype) void {
    var u8buf: [256]u8 = [_]u8{0} ** 256;
    _ = bufPrint(u8buf[0..], str, args) catch {
        return;
    };
    var u16buf: [256:0]u16 = [_:0]u16{0} ** 256;
    for (u8buf, 0..) |c, i| {
        u16buf[i] = c;
    }

    _ = uefi.system_table.con_out.?.outputString(u16buf[0..]);
}

pub fn clrscr() void {
    uefi.system_table.con_out.?.clearScreen() catch {};
}
