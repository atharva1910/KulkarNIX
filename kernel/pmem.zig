const serial = @import("serial.zig");
const MemoryDescriptor = @import("std").os.uefi.tables.MemoryDescriptor;
const MemoryType = @import("std").os.uefi.tables.MemoryType;
//const paging = @import("paging.zig");
var MemMapBitVec: []u8 = undefined;
var mmap: [*]MemoryDescriptor = undefined;
var dsize: usize = undefined;
var msize: usize = undefined;

pub fn Init(map: ?[*]MemoryDescriptor, mmapSize: usize, descSize: usize, totalPages: usize) void {
    mmap = map.?;
    dsize = descSize;
    msize = mmapSize;

    // 1bit = 1 Page ->  1Byte = 8 Pages
    const bytes4mem = totalPages >> 3;
    const p4mem = bytes4mem >> 12;
    serial.write("memory map {*} totalPages 0x{x} bytes 0x{x} p4mem 0x{x}\n", .{ mmap, totalPages, bytes4mem, p4mem });

    // Now we need to find a big enough "hole" in the memory map where we can fit this mem map bit fields
    // For qemu I know that the first 0xa0 pages are free and valid so thats what we will use
    var pMemMapBitVec: [*]u8 = @ptrFromInt(0x4040000000);
    MemMapBitVec = pMemMapBitVec[0..bytes4mem];
}

fn traverse_map(mmap: [*]align(4096) MemoryDescriptor, msize: usize, dsize: usize) traversed_map {
    var tmap: traversed_map = .{
        .min_addr = 0,
        .num_pages = 0,
        .total_pages = 0,
        .total_blocks = 0,
    };
    var itr: *MemoryDescriptor = @ptrCast(mmap);
    var clubbed: MemoryDescriptor = itr.*;
    const num_desc: usize = msize / dsize;
    var idx: usize = 0;

    while (idx < num_desc) : (itr = @ptrFromInt(@intFromPtr(mmap) + (idx * dsize))) {
        defer idx += 1;

        if (itr.type != MemoryType.boot_services_code and itr.type != MemoryType.boot_services_data and itr.type != MemoryType.conventional_memory) {
            //serial.write("skipping type {} pstart: 0x{x} num_pages: 0x{x} \n", .{ itr.type, itr.physical_start, itr.number_of_pages });
            continue;
        }

        tmap.total_pages += itr.number_of_pages;

        if (tmap.min_addr == 0 or itr.physical_start < tmap.min_addr) {
            tmap.min_addr = itr.physical_start;
            tmap.num_pages = itr.number_of_pages;
        }

        const clubbed_size = clubbed.number_of_pages << 12;
        const itr_size = itr.number_of_pages << 12;
        if (itr.physical_start < clubbed.physical_start + clubbed_size) {
            if (itr.physical_start + itr_size < clubbed.physical_start + clubbed_size) continue;
            clubbed.number_of_pages += (itr.physical_start + itr_size - clubbed.physical_start - clubbed_size) >> 12;
        } else if (itr.physical_start == clubbed.physical_start + clubbed_size) {
            clubbed.number_of_pages += itr.number_of_pages;
        } else {
            serial.write("pstart: 0x{x} num_pages: 0x{x} \n", .{ clubbed.physical_start, clubbed.number_of_pages });
            clubbed = itr.*;
        }
    }

    serial.write("total_pages 0x{x}\n", .{tmap.total_pages});
    return tmap;
}
