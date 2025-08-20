const serial = @import("serial.zig");
const MemoryDescriptor = @import("std").os.uefi.tables.MemoryDescriptor;
const MemoryType = @import("std").os.uefi.tables.MemoryType;
const paging = @import("paging.zig");
var mmap: [*]align(4096) MemoryDescriptor = undefined;
var dsize: usize = undefined;
var msize: usize = undefined;

const traversed_map = struct {
    min_addr: u64,
    num_pages: u64,
    total_pages: u64,
    total_blocks: u64,
};

pub fn init(map: ?[*]align(4096) u8, mmap_size: usize, desc_size: usize) void {
    mmap = @ptrCast(map.?);
    dsize = desc_size;
    msize = mmap_size;
    serial.write("memory map 0x{x} 0x{x} 0x{x}\n", .{ @intFromPtr(mmap), msize, desc_size });

    const tmap = traverse_map();
    serial.write("min_addr: 0x{x} total_pages: 0x{x}\n", .{ tmap.min_addr, tmap.total_pages });
    paging.init_kernel_pages(tmap.min_addr, tmap.total_pages);
}

fn traverse_map() traversed_map {
    var tmap: traversed_map = .{
        .min_addr = 0,
        .num_pages = 0,
        .total_pages = 0,
        .total_blocks = 0,
    };
    var itr: *MemoryDescriptor = @ptrCast(mmap);
    var clubbed: *MemoryDescriptor = itr;
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
            clubbed = itr;
        }
    }

    return tmap;
}
