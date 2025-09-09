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
        footer: *align(1) u64,
        size: usize,

        const HeapNodeIterator = struct {
            head: ?*HeapNode,
            itr: ?*HeapNode,

            pub fn next(self: *HeapNodeIterator) ?*HeapNode {
                if (self.itr == null) return null;
                const temp = self.itr;
                self.itr = self.itr.?.next;
                if (self.itr == self.head) {
                    self.itr = null;
                }
                return temp;
            }
        };

        pub fn NewNode(comptime T: type, start: T, n: usize) KError!*HeapNode {
            var node: *HeapNode = undefined;
            switch (@typeInfo(T)) {
                .pointer => {
                    node = @ptrCast(@alignCast(start));
                },

                .int => {
                    node = @ptrFromInt(start);
                },

                else => {
                    return KError.InvalidArg;
                },
            }

            node.next = node;
            node.prev = node;
            node.size = n - @sizeOf(usize);
            StampFooter(node);
            return node;
        }

        pub fn Iterator(self: *HeapNode) HeapNodeIterator {
            return .{
                .itr = self,
                .head = self,
            };
        }

        pub fn Print(self: *const HeapNode) void {
            Serial.Write("Heap Node {*}\n\tNext {*} Prev {*}\n\tSize 0x{x}\n\tFooter: {*} Footer Size: 0x{x}\n", .{
                self,
                self.next,
                self.prev,
                self.size,
                self.footer,
                self.footer.*,
            });
        }

        pub fn SplitNode(self: *HeapNode, n: usize) ![]u8 {
            if (self.size + @sizeOf(usize) < n) {
                return KError.NoMemory;
            }

            const ret: [*]u8 = @ptrCast(self.footer);
            self.size -= n;
            StampFooter(self);
            self.Print();
            return ret[0..n];
        }

        fn StampFooter(node: *HeapNode) void {
            node.footer = @ptrFromInt(@intFromPtr(node) + node.size);
            node.footer.* = node.size;
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
        self.HeapSize = 4 << 20;
        self.ListPtr = @ptrCast(@alignCast(try phyMemMgr.?.AllocPages(self.HeapSize >> 12)));
        self.List = try HeapNode.NewNode([]u8, self.ListPtr, self.HeapSize);
        self.List.?.Print();
    }

    pub fn alloc(self: *HeapManager, size: usize) ![]u8 {
        if (size == 0) {
            return KError.InvalidArg;
        }

        if (self.List == null) {
            return KError.NullPtr;
        }

        var itr = self.List.?.Iterator();
        while (itr.next()) |node| {
            if (node.size + @sizeOf(usize) <= size) continue;
            return try node.SplitNode(size);
        }

        return KError.NoMemory;
    }

    pub fn free(self: *HeapManager, p: []u8) !void {
        if (p.len == 0) {
            return KError.InvalidArg;
        }

        if (self.List == null) {
            return KError.NullPtr;
        }

        const node = try HeapNode.NewNode([]u8, p, p.len);
        var tail = self.List.?.prev;
        var head = self.List;

        node.prev = tail;
        node.next = head;

        head.?.prev = node;
        tail.?.next = node;

        self.List = node;
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
    Serial.Write("Heap Initialized: {*}\n", .{HeapMgr.List});
}
