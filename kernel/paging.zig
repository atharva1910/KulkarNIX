const Serial = @import("serial.zig");
const base_addr = 0x4040000000;
const KState = @import("kstate.zig").KState;

pub const PageTableMgr = struct {
    PageTables: [][512]u64,
    NumPDPT: usize,
    NumPDT: usize,
    NumPT: usize,
    TotalPages: usize,
    PageTablePages: usize,

    pub fn Print(self: *PageTableMgr) void {
        Serial.Write("PageTables {*}\nPDPT: 0x{x}\nPDT: 0x{x}\nPT: 0x{x}\nTotalPages: 0x{x}\n", .{
            self.PageTables,
            self.NumPDPT,
            self.NumPDT,
            self.NumPT,
            self.TotalPages,
        });
    }
};

var PTManager: PageTableMgr = undefined;

pub fn Init(mgr: PageTableMgr) void {
    PTManager = mgr;
    KState.SetPageTableManger(&PTManager);
    Serial.Write("Paging Initialized\n", .{});
}
