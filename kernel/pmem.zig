const serial = @import("serial.zig");
const MemoryDescriptor = @import("std").os.uefi.tables.MemoryDescriptor;
const MemoryType = @import("std").os.uefi.tables.MemoryType;
var mmap: [*]align(4096) MemoryDescriptor = undefined;
var dsize: usize = undefined;
var msize: usize = undefined;

pub fn init(map: ?[*]align(4096) u8, mmap_size: usize, desc_size: usize) void {
    mmap = @ptrCast(map.?);
    dsize = desc_size;
    msize = mmap_size;
    serial.write("memory map 0x{x} 0x{x} 0x{x}\n", .{ @intFromPtr(mmap), msize, desc_size });
    print_map();
}

fn print_map() void {
    var itr: *MemoryDescriptor = @ptrCast(mmap);
    var clubbed: *MemoryDescriptor = itr;
    const num_desc: usize = msize / dsize;
    var idx: usize = 0;
    while (idx < num_desc) : (itr = @ptrFromInt(@intFromPtr(mmap) + (idx * dsize))) {
        defer idx += 1;

        if (itr.type != MemoryType.boot_services_code and itr.type != MemoryType.boot_services_data and itr.type != MemoryType.conventional_memory)
            continue;

        const clubbed_size = clubbed.number_of_pages << 12;
        const itr_size = itr.number_of_pages << 12;
        if (itr.physical_start < clubbed.physical_start + clubbed_size) {
            if (itr.physical_start + itr_size < clubbed.physical_start + clubbed_size) continue;
            clubbed.number_of_pages += (itr.physical_start + itr_size - clubbed.physical_start - clubbed_size) >> 12;
        } else if (itr.physical_start == clubbed.physical_start + clubbed_size) {
            clubbed.number_of_pages += itr.number_of_pages;
        } else {
            serial.write("pstart: 0x{x} num_pages: 0x{x} \n", .{ clubbed.physical_start, clubbed.number_of_pages });
            clubbed = itr;
        }
    }
    serial.write("pstart: 0x{x} num_pages: 0x{x} \n", .{ clubbed.physical_start, clubbed.number_of_pages });
}
