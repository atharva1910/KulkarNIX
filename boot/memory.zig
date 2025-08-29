const uefi = @import("std").os.uefi;
const status = uefi.Status;
const elf = @import("std").elf;
const main = @import("main.zig");
const MemoryDescriptor = @import("std").os.uefi.tables.MemoryDescriptor;
const serial = @import("serial.zig");
const paging = @import("paging.zig");
const MemoryType = @import("std").os.uefi.tables.MemoryType;
const assert = @import("std").debug.assert;
const Page = uefi.Page;
const MemoryMapSlice = uefi.tables.MemoryMapSlice;

pub fn alloc_pages(pages: usize) ![]align(4096) Page {
    return try main.boot_services.allocatePages(
        uefi.tables.AllocateType.any,
        uefi.tables.MemoryType.boot_services_data,
        pages,
    );
}

pub fn free(buf: []align(8) u8) void {
    main.boot_services.freePool(buf.ptr) catch {};
}

pub fn alloc(size: usize) ![]align(8) u8 {
    return try main.boot_services.allocatePool(
        uefi.tables.MemoryType.boot_services_data,
        size,
    );
}

pub fn GetMemoryMap() !MemoryMapSlice {
    const mmapInfo = try main.boot_services.getMemoryMapInfo();
    const buf = try alloc(mmapInfo.len * mmapInfo.descriptor_size);
    return try main.boot_services.getMemoryMap(buf);
}

const traversed_map = struct {
    min_addr: u64,
    num_pages: u64,
    total_pages: u64,
    total_blocks: u64,
};

pub const clubbed_entry = struct {
    paddr: u64,
    num_pages: u64,
};

pub fn ClubMmap(mmap: [*]align(4096) MemoryDescriptor, msize: usize, dsize: usize, cmap: *[*]clubbed_entry) usize {
    const num_desc: usize = msize / dsize;
    var itr: *MemoryDescriptor = &mmap[0];
    var idx: usize = 0;
    var total_pages: u64 = 0;
    var total_clubbed: u32 = 0;
    var total_size: u64 = 0;

    while (idx < num_desc) : (itr = @ptrFromInt(@intFromPtr(mmap) + (idx * dsize))) {
        idx += 1;
        assert(itr.physical_start >= total_size);
        total_size += itr.number_of_pages << 12;
        total_pages += itr.number_of_pages;
        total_clubbed += 1;
    }

    if (status.success != alloc(total_clubbed * @sizeOf(clubbed_entry), @ptrCast(cmap))) {
        serial.write("Failed to allocate buffer for clubbed entries\r\n", .{});
        return 0;
    } else {
        serial.write("Mapping entire memory worth 0x{x} pages to 0x{x}\n", .{ total_size, paging.kMemAddr });
    }

    var pcmap: [*]clubbed_entry = @ptrCast(cmap.*);

    const num_pt = (total_pages >> 9) + 1;
    const num_pdt = (num_pt >> 9) + 1;
    const num_pdpt = (num_pdt >> 9) + 1;
    const num_pml4 = (num_pdpt >> 9) + 1;
    assert(num_pml4 == 1);

    idx = 0;
    itr = &mmap[0];
    var pcmap_idx: usize = 0;
    pcmap[pcmap_idx].paddr = itr.physical_start;
    pcmap[pcmap_idx].num_pages = itr.number_of_pages;

    while (idx < num_desc) : (itr = @ptrFromInt(@intFromPtr(mmap) + (idx * dsize))) {
        idx += 1;
        const itr_size = itr.number_of_pages << 12;
        const itr_end = itr.physical_start + itr_size;
        const clubbed_start = pcmap[pcmap_idx].paddr;
        const clubbed_end = clubbed_start + (pcmap[pcmap_idx].num_pages << 12);

        if (clubbed_end == itr.physical_start) {
            pcmap[pcmap_idx].num_pages += itr.number_of_pages;
            //serial.write("Combining  entry: pcmap[0x{x}] addr: 0x{x} numpages: 0x{x} += addr 0x{x} numpages 0x{x}\n", .{ pcmap_idx, pcmap[pcmap_idx].paddr, pcmap[pcmap_idx].num_pages, itr.physical_start, itr.number_of_pages });
            continue;
        }

        if (itr.physical_start >= clubbed_start and itr.physical_start <= clubbed_end) {
            const diff = (itr_end - clubbed_end) >> 12;
            //serial.write("Updating entry: pcmap[0x{x}] addr: 0x{x} numpages: 0x{x} -> numpages 0x{x}\n", .{ pcmap_idx, pcmap[pcmap_idx].paddr, pcmap[pcmap_idx].num_pages, pcmap[pcmap_idx].num_pages + diff });
            pcmap[pcmap_idx].num_pages += diff;
            continue;
        }

        pcmap_idx += 1;
        pcmap[pcmap_idx].paddr = itr.physical_start;
        pcmap[pcmap_idx].num_pages = itr.number_of_pages;
        //serial.write("New entry: addr: 0x{x} numpages: 0x{x}\n", .{ pcmap[pcmap_idx].paddr, pcmap[pcmap_idx].num_pages });
    }

    for (0..pcmap_idx + 1) |i| {
        serial.write("pcmap[{}] paddr 0x{x} num_pages 0x{x}\n", .{ i, pcmap[i].paddr, pcmap[i].num_pages });
    }

    return pcmap_idx + 1;
}
