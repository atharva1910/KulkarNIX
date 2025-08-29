const serial = @import("serial.zig");
const MemoryDescriptor = @import("std").os.uefi.tables.MemoryDescriptor;
const assert = @import("std").debug.assert;
const MemoryType = @import("std").os.uefi.tables.MemoryType;
const kMemAddr = 0x4040000000;
//const paging = @import("paging.zig");

var mmap: [*]MemoryDescriptor = undefined;
var dsize: usize = undefined;
var msize: usize = undefined;

const PageState = enum(u1) {
    FREE,
    ALLOCATED,
};
var MemMapBitVec: []PageState = undefined;

fn MarkPages(addr: usize, nPages: usize, state: PageState) void {
    for (0..nPages) |i| {
        const _addr = addr + (i << 12) - kMemAddr;
        assert(_addr & 0x1F == 0);
        assert(_addr >> 12 < MemMapBitVec.len);
        MemMapBitVec[_addr >> 12] = state;
    }
}

pub fn FreePage(addr: [*]u8) void {
    MarkPages(@intFromPtr(addr), 1, PageState.FREE);
}

pub fn AllocNumPages(nPages: u32) ?[*]u8 {
    var ws: usize = 0;
    outer: while (ws < MemMapBitVec.len) {
        if (MemMapBitVec.len - ws < nPages) {
            return null;
        }

        for (ws..MemMapBitVec.len) |we| {
            if (we - ws == nPages) {
                MarkPages(ws << 12, we - ws, PageState.ALLOCATED);
                return @ptrFromInt(kMemAddr + (ws << 12));
            }

            if (MemMapBitVec[we] != PageState.FREE) {
                ws = we + 1;
                continue :outer;
            }
        }
    }
    return null;
}

pub fn Init(map: ?[*]MemoryDescriptor, mmapSize: usize, descSize: usize, totalPages: usize) void {
    mmap = map.?;
    dsize = descSize;
    msize = mmapSize;

    const bytes4mem = totalPages >> 3;
    const p4mem = bytes4mem >> 12;
    serial.write("memory map {*} totalPages 0x{x} bytes 0x{x} p4mem 0x{x}\n", .{ mmap, totalPages, bytes4mem, p4mem });

    // Now we need to find a big enough "hole" in the memory map where we can fit this mem map bit fields
    // For qemu I know that the first 0xa0 pages are free and valid so thats what we will use
    var pMemMapBitVec: [*]PageState = @ptrFromInt(kMemAddr);
    MemMapBitVec = pMemMapBitVec[0 .. totalPages + 1];
    @memset(MemMapBitVec, PageState.FREE);
    MarkPages(kMemAddr, p4mem, PageState.ALLOCATED);
    InitMemBitVector();
}

fn InitMemBitVector() void {
    var itr: *MemoryDescriptor = &mmap[0];
    const num_desc: usize = msize / dsize;
    var idx: usize = 0;
    var total_pages: usize = 0;

    while (idx < num_desc) : (itr = @ptrFromInt(@intFromPtr(mmap) + (idx * dsize))) {
        defer idx += 1;

        //assert(itr.physical_start == (total_pages << 12) + kMemAddr);
        //serial.write("0x{x} == 0x{x}\n", .{ itr.physical_start, (total_pages << 12) });
        serial.write("mem block: 0x{x} type{} numpages {} totalnumpages 0x{x}\n", .{ itr.physical_start + kMemAddr, itr.type, itr.number_of_pages, total_pages });
        total_pages += itr.number_of_pages;

        if (itr.type == MemoryType.boot_services_code or
            itr.type == MemoryType.boot_services_data or
            itr.type == MemoryType.conventional_memory) continue;

        MarkPages(itr.physical_start + kMemAddr, itr.number_of_pages, PageState.ALLOCATED);
    }
}
