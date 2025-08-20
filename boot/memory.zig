const uefi = @import("std").os.uefi;
const status = uefi.Status;
const elf = @import("std").elf;
const main = @import("main.zig");
const MemoryDescriptor = @import("std").os.uefi.tables.MemoryDescriptor;
const serial = @import("serial.zig");
const paging = @import("paging.zig");
const MemoryType = @import("std").os.uefi.tables.MemoryType;
const assert = @import("std").debug.assert;

pub fn alloc_pages(pages: usize, buf: *[*]align(4096) u8) uefi.Status {
    return main.boot_services.allocatePages(
        uefi.tables.AllocateType.allocate_any_pages,
        uefi.tables.MemoryType.boot_services_data,
        pages,
        buf,
    );
}

pub fn free(buf: [*]align(8) u8) void {
    _ = main.boot_services.freePool(buf);
}

pub fn alloc(size: usize, buf: *[*]align(8) u8) uefi.Status {
    return main.boot_services.allocatePool(
        uefi.tables.MemoryType.boot_services_data,
        size,
        buf,
    );
}

pub fn GetMemoryMap(size: *usize, buf: *[*]align(4096) MemoryDescriptor, key: *usize, descSize: *usize, descVer: *u32) uefi.Status {
    var sRet = main.boot_services.getMemoryMap(
        size,
        @ptrCast(buf.*),
        key,
        descSize,
        descVer,
    );

    if (sRet == status.success) {
        serial.write("wrong ret from getMemoryMap {}, size: {}\r\n", .{ sRet, size.* });
        return status.unsupported;
    }

    const num_pages = (size.* >> 12) + 1;
    sRet = alloc_pages(num_pages, @ptrCast(buf));
    if (sRet != status.success) {
        serial.write("Failed to allocate buffer\r\n", .{});
        return sRet;
    }

    sRet = main.boot_services.getMemoryMap(
        size,
        buf.*,
        key,
        descSize,
        descVer,
    );

    if (sRet != status.success) {
        serial.write("getMemoryMap failed\r\n", .{});
        return sRet;
    }

    //serial.write("Memory map stored in page {*} num_pages: {}, size: {}\r\n", .{ buf.*, num_pages, size.* });
    return sRet;
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
