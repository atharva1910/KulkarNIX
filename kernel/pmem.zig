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

const PageState = enum(u1) {
    FREE,
    ALLOCATED,
};

const PMemNode = struct {
    next: ?*PMemNode,
    prev: ?*PMemNode,
    pages: usize,
    start: usize,
    end: usize,

    pub fn NewNode(comptime T: type, start: usize, n: usize) !*PMemNode {
        var node: *PMemNode = undefined;
        switch (@typeInfo(T)) {
            .pointer => {
                node = @ptrCast(start);
                node.start = @intFromPtr(start);
            },

            .int => {
                node = @ptrFromInt(start);
                Serial.Write(" create a new node: {*}\n", .{node});
                node.start = start;
            },

            else => {
                return KError.InvalidArg;
            },
        }
        Serial.Write(" create a new node: 0x{x}\n", .{start});
        node.next = node;
        node.prev = node;
        node.pages = n;
        node.end = start + (n << 12);
        return node;
    }
};

pub const PMemManager = struct {
    list: ?*PMemNode,

    pub fn AddNode(self: *PMemManager, start: usize, n: usize) !void {
        const node = PMemNode.NewNode(usize, start, n) catch |err| {
            Serial.Write("Failed to create a new node: {}\n", .{err});
            return err;
        };

        if (self.list == null) {
            self.list = node;
            return;
        }

        node.next = self.list;
        node.prev = self.list.?.prev;
        self.list = node;
    }

    pub fn loop(self: *PMemManager) void {
        if (self.list == null) return;

        var head = self.list;

        while (true) {
            Serial.Write("Node {*} nPages {}\n", .{ head.?, head.?.pages });
            head = head.?.next;
            if (head == self.list) break;
        }
    }
};

//MemMapBitVec: []PageState,
//KMemStart: usize,
//mmap: ?MemoryMapSlice,
//
//pub fn MarkPages(self: *Self, addr: usize, pages: usize, state: PageState) KError!void {
//    assert(addr > self.KMemStart);
//    assert(pages > 0);
//    assert(addr & 0x1F == 0);
//    const start = (addr - self.KMemStart) >> 12;
//    const end = start + pages;
//    @memset(self.MemMapBitVec[start..end], state);
//}
//
//fn InitMemBitVector(self: *Self, maxAddr: usize) KError!void {
//    if (self.mmap == null) {
//        //Serial.Write("{}:{} mmap is null\n", .{ @src().fn_name, @src().line });
//        return KError.NullPtr;
//    }
//
//    var itr = self.mmap.?.iterator();
//
//    while (true) {
//        const desc = itr.next();
//        if (desc == null) break;
//        // TODO: Check if overlaps
//        if (desc.?.physical_start > maxAddr) break;
//        if (desc.?.type == MemoryType.boot_services_code or
//            desc.?.type == MemoryType.boot_services_data or
//            desc.?.type == MemoryType.conventional_memory)
//        {
//            continue;
//        }
//
//        try self.MarkPages(desc.?.physical_start + kMemAddr, desc.?.number_of_pages, PageState.ALLOCATED);
//    }
//}
//
//pub fn FreePages(self: *Self, addr: []Page) void {
//    self.MarkPages(@intFromPtr(addr.ptr), addr.len, PageState.FREE);
//}
//
//pub fn AllocPages(self: *Self, nPages: u32) ?[]Page {
//    var ws: usize = 0;
//    outer: while (ws < self.MemMapBitVec.len) {
//        if (self.MemMapBitVec.len - ws < nPages) {
//            return null;
//        }
//
//        for (ws..self.MemMapBitVec.len) |we| {
//            if (we - ws == nPages) {
//                MarkPages(ws << 12, we - ws, PageState.ALLOCATED);
//                return @ptrFromInt(kMemAddr + (ws << 12));
//            }
//
//            if (self.MemMapBitVec[we] != PageState.FREE) {
//                ws = we + 1;
//                continue :outer;
//            }
//        }
//    }
//    return null;
//}
//
//pub fn Init1(
//    mmapSlice: MemoryMapSlice,
//    totalPages: usize,
//    KernelStart: usize,
//    KernelPages: usize,
//) KError!Self {
//    const bytes4mem = totalPages >> 3;
//    const p4mem = bytes4mem >> 12;
//    const maxAddr = (totalPages << 12) - 1;
//
//    var PMEM = Self{
//        .KMemStart = kMemAddr,
//        .MemMapBitVec = undefined,
//        .mmap = mmapSlice,
//    };
//
//    // Now we need to find a big enough "hole" in the memory map where we can fit this mem map bit fields
//    // For qemu I know that the first 0xa0 pages are free and valid so thats what we will use
//    var pMemMapBitVec: [*]PageState = @ptrFromInt(kMemAddr);
//    PMEM.MemMapBitVec = pMemMapBitVec[0 .. totalPages + 1];
//    @memset(PMEM.MemMapBitVec, PageState.FREE);
//
//    // Mark the bits which are reserved for Page Bit Map
//    @memset(PMEM.MemMapBitVec[0..p4mem], PageState.ALLOCATED);
//
//    // Mark all the unusable pages as Allocated until maxaddr
//    PMEM.InitMemBitVector(maxAddr) catch |err| {
//        Serial.Write("Failed to init mem bit vector: {}\n", .{err});
//        return err;
//    };
//
//    // Mark the kernel code pages as allocated
//    PMEM.MarkPages(KernelStart, KernelPages, PageState.ALLOCATED) catch |err| {
//        Serial.Write("Failed to Mark Kernel address as allocated: {}\n", .{err});
//        return err;
//    };
//
//    return PMEM;
//}
//
pub fn Init(
    mmapSlice: MemoryMapSlice,
    totalPages: usize,
    KernelStart: usize,
    KernelPages: usize,
) KError!void {
    _ = KernelPages + KernelStart;
    const maxAddr = (totalPages << 12) - 1;
    Serial.Write("Max Addr: 0x{x}\n", .{maxAddr});
    const PMemMgr = KState.GetPhyMemMgr();
    if (PMemMgr == null) {
        return KError.NullPtr;
    }

    var itr = mmapSlice.iterator();
    while (itr.next()) |desc| {
        if (desc.physical_start > maxAddr) {
            return;
        }

        if (desc.type == MemoryType.boot_services_code or
            desc.type == MemoryType.boot_services_data or
            desc.type == MemoryType.conventional_memory)
        {
            Serial.Write("paddr 0x{x} npages 0x{x} vaddr 0x{x}\n", .{ desc.physical_start, desc.number_of_pages, desc.physical_start + kMemAddr });
            try PMemMgr.?.AddNode(desc.physical_start + kMemAddr, desc.number_of_pages);
        }
    }

    //const bytes4mem = totalPages >> 3;
    //const p4mem = bytes4mem >> 12;
    //
    //var PMEM = Self{
    //    .KMemStart = kMemAddr,
    //    .MemMapBitVec = undefined,
    //    .mmap = mmapSlice,
    //};
    //
    //// Now we need to find a big enough "hole" in the memory map where we can fit this mem map bit fields
    //// For qemu I know that the first 0xa0 pages are free and valid so thats what we will use
    //var pMemMapBitVec: [*]PageState = @ptrFromInt(kMemAddr);
    //PMEM.MemMapBitVec = pMemMapBitVec[0 .. totalPages + 1];
    //@memset(PMEM.MemMapBitVec, PageState.FREE);
    //
    //// Mark the bits which are reserved for Page Bit Map
    //@memset(PMEM.MemMapBitVec[0..p4mem], PageState.ALLOCATED);
    //
    //// Mark all the unusable pages as Allocated until maxaddr
    //PMEM.InitMemBitVector(maxAddr) catch |err| {
    //    Serial.Write("Failed to init mem bit vector: {}\n", .{err});
    //    return err;
    //};
    //
    //// Mark the kernel code pages as allocated
    //PMEM.MarkPages(KernelStart, KernelPages, PageState.ALLOCATED) catch |err| {
    //    Serial.Write("Failed to Mark Kernel address as allocated: {}\n", .{err});
    //    return err;
    //};
    //
    //return PMEM;
}
