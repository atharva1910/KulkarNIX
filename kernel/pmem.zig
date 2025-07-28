const serial = @import("serial.zig");
const MemoryDescriptor = @import("std").os.uefi.tables.MemoryDescriptor;
const MemoryType = @import("std").os.uefi.tables.MemoryType;
const paging = @import("paging.zig");
var mmap: [*]align(4096) MemoryDescriptor = undefined;
var dsize: usize = undefined;
var msize: usize = undefined;

pub fn init(map: ?[*]align(4096) u8, mmap_size: usize, desc_size: usize) void {
    mmap = @ptrCast(map.?);
    dsize = desc_size;
    msize = mmap_size;
    serial.write("memory map 0x{x} 0x{x} 0x{x}\n", .{ @intFromPtr(mmap), msize, desc_size });

    var min_addr: u64 = 0;
    var max_addr: u64 = 0;
    var num_pages: u64 = 0;
    traverse_map(&min_addr, &max_addr, &num_pages);
    serial.write("min_addr: 0x{x} max_addr: 0x{x} total_pages: 0x{x}\n", .{ min_addr, max_addr, num_pages });
    paging.init(min_addr, max_addr, num_pages);
}

fn traverse_map(min_addr: *u64, max_addr: *u64, num_pages: *u64) void {
    var itr: *MemoryDescriptor = @ptrCast(mmap);
    var clubbed: *MemoryDescriptor = itr;
    const num_desc: usize = msize / dsize;
    var idx: usize = 0;

    while (idx < num_desc) : (itr = @ptrFromInt(@intFromPtr(mmap) + (idx * dsize))) {
        defer idx += 1;

        if (itr.type != MemoryType.boot_services_code and itr.type != MemoryType.boot_services_data and itr.type != MemoryType.conventional_memory) {
            //serial.write("skipping type {} pstart: 0x{x} num_pages: 0x{x} \n", .{ itr.type, itr.physical_start, itr.number_of_pages });
            continue;
        } else {
            //serial.write("type {} pstart: 0x{x} num_pages: 0x{x} \n", .{ itr.type, itr.physical_start, itr.number_of_pages });
            num_pages.* += itr.number_of_pages;
        }

        min_addr.* = @min(min_addr.*, itr.physical_start);

        const clubbed_size = clubbed.number_of_pages << 12;
        const itr_size = itr.number_of_pages << 12;
        if (itr.physical_start < clubbed.physical_start + clubbed_size) {
            if (itr.physical_start + itr_size < clubbed.physical_start + clubbed_size) continue;
            clubbed.number_of_pages += (itr.physical_start + itr_size - clubbed.physical_start - clubbed_size) >> 12;
        } else if (itr.physical_start == clubbed.physical_start + clubbed_size) {
            clubbed.number_of_pages += itr.number_of_pages;
        } else {
            //serial.write("pstart: 0x{x} num_pages: 0x{x} \n", .{ clubbed.physical_start, clubbed.number_of_pages });
            clubbed = itr;
        }
    }

    max_addr.* = clubbed.physical_start + (clubbed.number_of_pages << 12);
    //serial.write("pstart: 0x{x} num_pages: 0x{x} \n", .{ clubbed.physical_start, clubbed.number_of_pages });
}
