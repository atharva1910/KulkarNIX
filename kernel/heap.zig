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
    List: ?*HeapNode,

    const HeapNode = struct {
        next: ?*HeapNode,
        prev: ?*HeapNode,
        pages: usize,
        start: usize,
        end: usize,

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

        pub fn NewNode(comptime T: type, start: usize, n: usize) KError!*HeapNode {
            var node: *HeapNode = undefined;
            switch (@typeInfo(T)) {
                .pointer => {
                    node = @ptrCast(start);
                    node.start = @intFromPtr(start);
                },

                .int => {
                    node = @ptrFromInt(start);
                    node.start = start;
                    node.pages = n;
                },

                else => {
                    return KError.InvalidArg;
                },
            }

            node.next = null;
            node.prev = null;
            node.pages = n;
            node.end = start + (n << 12);
            return node;
        }

        pub fn Iterator(self: *HeapNode) HeapNodeIterator {
            return .{
                .itr = self,
                .head = self,
            };
        }

        pub fn Print(self: *const HeapNode) void {
            Serial.Write("Node {*}\n\tNext {*} Prev {*}\n\tPages 0x{x} Start 0x{x} End 0x{x}\n", .{
                self,
                self.next,
                self.prev,
                self.pages,
                self.start,
                self.end,
            });
        }

        pub fn TrimPages(self: *HeapNode, n: usize) ![*]u8 {
            if (self.pages < n) {
                return KError.NoMemory;
            }

            self.end -= n << 12;
            self.pages -= n;
            return @ptrFromInt(self.end);
        }
    };

    pub fn Init(self: *HeapManager) void {
        self.HeapSize = 0;
        self.List = null;
    }

    pub fn alloc(size: usize) ?[]u8 {
        return null;
    }

    pub fn free(p: []u8) void {}
};

var HeapMgr: HeapManager = undefined;

pub fn Init() void {
    HeapMgr.Init();
    KState.SetHeapManager(&HeapMgr);
}
