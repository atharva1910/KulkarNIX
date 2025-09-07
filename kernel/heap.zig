const Serial = @import("serial.zig");
const std = @import("std");
const assert = std.debug.assert;

const uefi = std.os.uefi;
const Page = uefi.Page;
const MemoryDescriptor = uefi.tables.MemoryDescriptor;
const MemoryType = uefi.tables.MemoryType;
const MemoryMapSlice = uefi.tables.MemoryMapSlice;
const KState = @import("kstate.zig").KState;
const Type = @import("std").builtin.Type;
const kMemAddr = 0x4040000000;
const KError = @import("kerrors.zig").KError;
const PageTableMgr = @import("paging.zig").PageTableMgr;

pub const HeapManager = struct {
    HeapSize: usize,
    HeapUsedSize: usize,
    ListPtr: []u8,
    List: ?*HeapNode,

    const HeapNode = struct {
        next: ?*HeapNode,
        prev: ?*HeapNode,
        start: usize,
        end: usize,
        size: usize,

        const HeapNodeIterator = struct {
            head: ?*HeapNode,
            itr: ?*HeapNode,

            pub fn next(self: *HeapNodeIterator) ?*HeapNode {
                const temp = self.itr;
                self.itr = self.itr.?.next;
                if (self.itr == null or self.itr.? == self.head.?) {
                    return null;
                } else {
                    return temp;
                }
            }
        };

        pub fn NewNode(comptime T: type, start: T, n: usize) KError!*HeapNode {
            var node: *HeapNode = undefined;
            switch (@typeInfo(T)) {
                .pointer => {
                    node = @ptrCast(@alignCast(start));
                    node.start = @intFromPtr(node);
                },

                .int => {
                    node = @ptrFromInt(start);
                    node.start = n;
                },

                else => {
                    return KError.InvalidArg;
                },
            }

            node.next = null;
            node.prev = null;
            node.size = n;
            node.end = node.start + node.size;
            return node;
        }

        pub fn Iterator(self: *HeapNode) HeapNodeIterator {
            return .{
                .itr = self,
                .head = self,
            };
        }

        pub fn Print(self: *const HeapNode) void {
            Serial.Write("Heap Node {*}\n\tNext {*} Prev {*}\n\tSize 0x{x}\n", .{
                self,
                self.next,
                self.prev,
                self.size,
            });
        }

        pub fn SplitNode(self: *HeapNode, n: usize) ![*]u8 {
            if (self.size < n) {
                return KError.NoMemory;
            }

            self.end -= n << 12;
            self.size -= n;
            return @ptrFromInt(self.end);
        }
    };

    pub fn Init(self: *HeapManager) !void {
        self.HeapSize = 0;
        self.HeapUsedSize = 0;
        self.List = null;

        const phyMemMgr = KState.GetPhyMemMgr();
        if (phyMemMgr == null) {
            return KError.NullPtr;
        }

        // Allocate 4 MB
        self.HeapSize = 4 << 8;
        self.ListPtr = try phyMemMgr.?.AllocPages(self.HeapSize);
        self.List = try HeapNode.NewNode([]u8, self.ListPtr, self.HeapSize);
        self.List.?.Print();
    }

    pub fn alloc(self: *HeapManager, size: usize) ![]u8 {
        if (size == 0) {
            return KError.InvalidArg;
        }

        if (self.HeapUsedSize + size > self.HeapSize) {
            return KError.NoMemory;
        }

        if (self.List == null) {
            return KError.NullPtr;
        }

        var itr = self.List.?.Iterator();
        while (itr.next()) |node| {
            if (node.size <= size + @sizeOf(HeapNode)) continue;
            return try node.SplitNode(size);
        }
        return KError.NoMemory;
    }

    pub fn free(p: []u8) void {
        if (p.len == 0) {
            return KError.InvalidArg;
        }
    }

    pub fn Loop(self: *HeapManager) void {
        if (self.List == null) {
            return;
        }

        var itr = self.List.?.Iterator();
        while (itr.next()) |node| {
            node.Print();
        }
    }
};

var HeapMgr: HeapManager = undefined;

pub fn Init() !void {
    try HeapMgr.Init();
    KState.SetHeapManager(&HeapMgr);
    Serial.Write("Heap Initialized: {*}", .{HeapMgr.List});
    HeapMgr.Loop();
}
