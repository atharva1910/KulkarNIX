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

pub fn VA2PA(addr: usize) usize {
    return addr - kMemAddr;
}

pub fn PA2VA(addr: usize) usize {
    return addr + kMemAddr;
}

pub const PMemManager = struct {
    List: ?*PMemNode,
    KernelStart: usize,
    KernelEnd: usize,
    PagingStart: usize,
    PagingEnd: usize,

    const PMemNode = struct {
        next: ?*PMemNode,
        prev: ?*PMemNode,
        pages: usize,
        start: usize,
        end: usize,

        const PMemNodeIterator = struct {
            head: ?*PMemNode,
            itr: ?*PMemNode,

            pub fn next(self: *PMemNodeIterator) ?*PMemNode {
                const temp = self.itr;
                self.itr = self.itr.?.next;
                if (self.itr == null or self.itr.? == self.head.?) {
                    return null;
                } else {
                    return temp;
                }
            }
        };

        pub fn NewNode(comptime T: type, start: usize, n: usize) KError!*PMemNode {
            var node: *PMemNode = undefined;
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

        pub fn Iterator(self: *PMemNode) PMemNodeIterator {
            return .{
                .itr = self,
                .head = self,
            };
        }

        pub fn Print(self: *const PMemNode) void {
            Serial.Write("PMem Node {*}\n\tNext {*} Prev {*}\n\tPages 0x{x} Start 0x{x} End 0x{x}\n", .{
                self,
                self.next,
                self.prev,
                self.pages,
                self.start,
                self.end,
            });
        }

        pub fn TrimPages(self: *PMemNode, n: usize) ![]Page {
            if (self.pages < n) {
                return KError.NoMemory;
            }

            self.end -= n << 12;
            self.pages -= n;

            const ret: [*]Page = @ptrFromInt(self.end);
            return ret[0..n];
        }
    };
    const OverlapType = enum {
        NO_OVERLAP,
        KERN_OVERLAP,
        PAGE_OVERLAP,
        BOTH_OVERLAP,
    };

    fn IsMemOverlapping(self: *PMemManager, m1Start: usize, m1End: usize) OverlapType {
        var kernOverlap = false;
        var pageOverlap = false;

        if (self.PagingStart >= m1Start and self.PagingStart < m1End) {
            pageOverlap = true;
        } else if (self.PagingEnd > m1Start and self.PagingEnd < m1End) {
            pageOverlap = true;
        }

        if (self.KernelStart >= m1Start and self.KernelStart < m1End) {
            kernOverlap = true;
        } else if (self.KernelEnd > m1Start and self.KernelEnd < m1End) {
            kernOverlap = true;
        }

        if (pageOverlap and kernOverlap) {
            return OverlapType.BOTH_OVERLAP;
        } else if (pageOverlap) {
            return OverlapType.PAGE_OVERLAP;
        } else if (kernOverlap) {
            return OverlapType.KERN_OVERLAP;
        }

        return OverlapType.NO_OVERLAP;
    }

    pub fn AddNode(self: *PMemManager, start: usize, n: usize) KError!void {
        const end = start + (n << 12);
        const ot = self.IsMemOverlapping(start, end);

        if (ot == OverlapType.NO_OVERLAP) {
            const node = PMemNode.NewNode(usize, start, n) catch |err| {
                Serial.Write("Failed to create a new node: {}\n", .{err});
                return err;
            };

            if (self.List == null) {
                self.List = node;
                return;
            }

            node.next = self.List;
            self.List.?.prev = node;
            self.List = node;
            return;
        }

        Serial.Write("Found overlapping chunk start 0x{x} type {}\n", .{ start, ot });
        try self.SplitChunk(ot, start, end);
    }

    pub fn Loop(self: *PMemManager) void {
        if (self.List == null) {
            return;
        }

        var itr = self.List.?.Iterator();
        while (itr.next()) |node| {
            node.Print();
        }
    }

    pub fn SplitChunk(
        self: *PMemManager,
        ot: OverlapType,
        m1Start: usize,
        m1End: usize,
    ) KError!void {
        var splitStart: usize = 0;
        var splitEnd: usize = 0;

        switch (ot) {
            OverlapType.KERN_OVERLAP => {
                // Dont support memory spannign multiple pages yet
                assert(self.KernelEnd < m1End);
                assert(m1Start <= self.KernelStart);
                splitStart = self.KernelStart;
                splitEnd = self.KernelEnd;
            },

            OverlapType.BOTH_OVERLAP, OverlapType.PAGE_OVERLAP => {
                assert(self.PagingEnd < m1End);
                assert(m1Start <= self.PagingStart);
                splitStart = self.PagingStart;
                splitEnd = self.PagingEnd;
            },

            else => assert(true),
        }

        const s1Pages = (splitStart - m1Start) >> 12;
        if (s1Pages > 0) {
            Serial.Write("First Split Start: 0x{x} nPages: 0x{x}\n", .{ m1Start, s1Pages });
            self.AddNode(m1Start, s1Pages) catch |err| {
                Serial.Write("Failed to split node start 0x{x} end 0x{x} into pages 0x{x} (second chunk)\n", .{ m1Start, m1End, s1Pages });
                return err;
            };
        }

        const s2Pages = (m1End - splitEnd) >> 12;
        if (s2Pages > 0) {
            Serial.Write("Second Split Start: 0x{x} nPages: 0x{x}\n", .{ splitEnd, s2Pages });
            self.AddNode(splitEnd, s2Pages) catch |err| {
                Serial.Write("Failed to split node start 0x{x} end 0x{x} into pages 0x{x} (second chunk)\n", .{ m1Start, m1End, s2Pages });
                return err;
            };
        }
    }

    pub fn AllocPages(self: *PMemManager, n: usize) ![]Page {
        if (self.List == null) {
            return KError.NullPtr;
        }

        var itr = self.List.?.Iterator();
        while (itr.next()) |node| {
            if (node.pages > n) {
                const mem = try node.TrimPages(n);
                return mem[0..n];
            }
        }

        return KError.NoMemory;
    }

    pub fn FreePages(self: *PMemManager, addr: []u8) !void {
        if (self.List == null or addr.len == 0) {
            return KError.NullPtr;
        }

        try self.AddNode(@intFromPtr(addr.ptr), addr.len >> 12);
    }

    pub fn Init(
        self: *PMemManager,
        KernelStart: usize,
        KernelEnd: usize,
        PagingStart: usize,
        PagingEnd: usize,
    ) void {
        self.KernelStart = KernelStart;
        self.KernelEnd = KernelEnd;
        self.PagingStart = PagingStart;
        self.PagingEnd = PagingEnd;
        self.List = null;
    }
};

var PMemMgr: PMemManager = undefined;

pub fn Init(
    mmapSlice: MemoryMapSlice,
    PageTableManager: PageTableMgr,
    KernelStart: usize,
    KernelPages: usize,
) KError!void {
    const KernelEnd = (KernelPages << 12) + KernelStart;
    const maxAddr = (PageTableManager.TotalPages << 12);

    const pagingStart = @intFromPtr(PageTableManager.PageTables.ptr);
    const pagingEnd = pagingStart + (PageTableManager.PageTablePages << 12);

    Serial.Write("Page Table [0x{x} - 0x{x}] Kernel [0x{x} - 0x{x}]\n", .{ pagingStart, pagingEnd, KernelStart, KernelEnd });
    PMemMgr.Init(KernelStart, KernelEnd, pagingStart, pagingEnd);
    KState.SetPhyMemMgr(&PMemMgr);

    var itr = mmapSlice.iterator();
    while (itr.next()) |desc| {
        if (desc.physical_start > maxAddr) {
            break;
        }

        if (desc.type != MemoryType.boot_services_code and
            desc.type != MemoryType.boot_services_data and
            desc.type != MemoryType.conventional_memory)
        {
            continue;
        }

        //Serial.Write("paddr 0x{x} npages 0x{x} vaddr 0x{x}\n", .{ desc.physical_start, desc.number_of_pages, desc.physical_start + kMemAddr });
        try PMemMgr.AddNode(PA2VA(desc.physical_start), desc.number_of_pages);
    }
}
