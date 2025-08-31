const Serial = @import("serial.zig");
const std = @import("std");
const assert = std.debug.assert;

const uefi = std.os.uefi;
const Page = uefi.Page;
const MemoryDescriptor = uefi.tables.MemoryDescriptor;
const MemoryType = uefi.tables.MemoryType;
const MemoryMapSlice = uefi.tables.MemoryMapSlice;

const kMemAddr = 0x4040000000;
const KError = @import("kerrors.zig").KError;
const Self = @This();
const PageState = enum(u1) {
    FREE,
    ALLOCATED,
};

MemMapBitVec: []PageState,
KMemStart: usize,
mmap: ?MemoryMapSlice,

pub fn MarkPages(self: *Self, addr: usize, pages: usize, state: PageState) KError!void {
    assert(addr > self.KMemStart);
    assert(pages > 0);
    assert(addr & 0x1F == 0);
    const start = (addr - self.KMemStart) >> 12;
    const end = start + pages;
    @memset(self.MemMapBitVec[start..end], state);
}

fn InitMemBitVector(self: *Self) KError!void {
    if (self.mmap == null) {
        //Serial.Write("{}:{} mmap is null\n", .{ @src().fn_name, @src().line });
        return KError.NullPtr;
    }

    var itr = self.mmap.?.iterator();
    while (true) {
        const desc = itr.next();
        if (desc == null) break;

        if (desc.?.type == MemoryType.boot_services_code or
            desc.?.type == MemoryType.boot_services_data or
            desc.?.type == MemoryType.conventional_memory)
        {
            continue;
        }

        try self.MarkPages(desc.?.physical_start + kMemAddr, desc.?.number_of_pages, PageState.ALLOCATED);
    }
}

pub fn FreePages(self: *Self, addr: []Page) void {
    self.MarkPages(@intFromPtr(addr.ptr), addr.len, PageState.FREE);
}

pub fn AllocPages(self: *Self, nPages: u32) ?[]Page {
    var ws: usize = 0;
    outer: while (ws < self.MemMapBitVec.len) {
        if (self.MemMapBitVec.len - ws < nPages) {
            return null;
        }

        for (ws..self.MemMapBitVec.len) |we| {
            if (we - ws == nPages) {
                MarkPages(ws << 12, we - ws, PageState.ALLOCATED);
                return @ptrFromInt(kMemAddr + (ws << 12));
            }

            if (self.MemMapBitVec[we] != PageState.FREE) {
                ws = we + 1;
                continue :outer;
            }
        }
    }
    return null;
}

pub fn Init(mmapSlice: MemoryMapSlice, totalPages: usize, KernelStart: usize, KernelPages: usize) KError!Self {
    const bytes4mem = totalPages >> 3;
    const p4mem = bytes4mem >> 12;

    var PMEM = Self{
        .KMemStart = kMemAddr,
        .MemMapBitVec = undefined,
        .mmap = mmapSlice,
    };

    // Now we need to find a big enough "hole" in the memory map where we can fit this mem map bit fields
    // For qemu I know that the first 0xa0 pages are free and valid so thats what we will use
    var pMemMapBitVec: [*]PageState = @ptrFromInt(kMemAddr);
    PMEM.MemMapBitVec = pMemMapBitVec[0 .. totalPages + 1];
    @memset(PMEM.MemMapBitVec, PageState.FREE);

    // Mark the bits which are reserved for Page Bit Map
    @memset(PMEM.MemMapBitVec[0..p4mem], PageState.ALLOCATED);

    PMEM.InitMemBitVector() catch |err| {
        Serial.Write("Failed to init mem bit vector: {}\n", .{err});
        return err;
    };

    PMEM.MarkPages(KernelStart, KernelPages, PageState.ALLOCATED) catch |err| {
        Serial.Write("Failed to Mark Kernel address as allocated: {}\n", .{err});
        return err;
    };

    return PMEM;
}
