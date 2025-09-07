const serial = @import("serial.zig");
const maxInt = @import("std").math.maxInt;
const base_addr = 0x4040000000;
const KState = @import("kstate.zig").KState;

pub const PageTableMgr = struct {
    PageTables: [][512]u64,
    NumPDPT: usize,
    NumPDT: usize,
    NumPT: usize,
    TotalPages: usize,
    PageTablePages: usize,
};

var PTManager: PageTableMgr = undefined;

pub fn Init(mgr: PageTableMgr) void {
    PTManager = mgr;
    KState.SetPageTableManger(&PTManager);
}
