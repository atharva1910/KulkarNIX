const serial = @import("serial.zig");
const MemoryDescriptor = @import("std").os.uefi.tables.MemoryDescriptor;
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
    const num_desc: usize = msize / dsize;
    for (0..num_desc) |i| {
        serial.write("type:{} pstart: 0x{x} vstart: 0x{x} num_pages: 0x{x} \n", .{ @intFromEnum(itr.type), itr.physical_start, itr.virtual_start, itr.number_of_pages });
        itr = @ptrFromInt(@intFromPtr(mmap) + (i * dsize));
    }
}
