const uefi = @import("std").os.uefi;
const serial = @import("serial.zig");
const assert = @import("std").debug.assert;
const status = uefi.Status;
const mem = @import("memory.zig");
const screen = @import("screen.zig");
const maxInt = @import("std").math.maxInt;
const Page = uefi.Page;
const MemoryDescriptor = @import("std").os.uefi.tables.MemoryDescriptor;
const PageType = enum(u2) { PML4, PDPT, PDT, PT };
const MemoryMapSlice = uefi.tables.MemoryMapSlice;
const Error = uefi.Error;
pub const kCompAddr = 0x4000000000;
pub const kMemAddr = 0x4040000000;
pub var pml4: ?[*]u64 = undefined;
pub var totalMemPages: usize = undefined;

fn DumpTable(table: [*]u64, lvl: u16) void {
    var tname: []const u8 = undefined;
    switch (lvl) {
        0 => tname = "PML4",
        1 => tname = "PDPT",
        2 => tname = "PDT",
        3 => tname = "PT",
        else => return,
    }

    for (0..512) |i| {
        if (table[i] == 0) continue;
        serial.write("{s}[{*}] -> [{}] 0x{x}\r\n", .{ tname, table, i, table[i] });
        if (lvl < 3)
            DumpTable(@ptrFromInt(table[i] & ~@as(u64, 0x3)), lvl + 1);
    }
}

pub fn DumpPageMap() void {
    if (pml4 == null) {
        serial.write("Base page table pml4 not allocated\r\n", .{});
        return;
    }
    DumpTable(pml4.?, 0);
}

pub fn IdentityMapPages(buffer: [*]u8, num_pages: u64) uefi.Status {
    var sRet: uefi.Status = status.success;
    var addr = @intFromPtr(buffer);
    for (0..num_pages) |_| {
        sRet = IdentityMapPage(addr);
        assert(sRet != status.success);
        addr += 0x1000;
    }

    return sRet;
}

pub fn IdentityMapPage(addr: usize) uefi.Status {
    var sRet: uefi.Status = status.load_error;
    var page: [*]align(4096) u64 = undefined;
    var table: [*]align(4096) u64 = undefined;
    if (pml4 == null) {
        //serial.write("Base page table pml4 not allocated\r\n", .{});
        return sRet;
    }

    const pml4_idx = (addr >> 39) & maxInt(u9);
    const pdpt_idx = (addr >> 30) & maxInt(u9);
    const pdt_idx = (addr >> 21) & maxInt(u9);
    const pt_idx = (addr >> 12) & maxInt(u9);

    if (pml4_idx != 0) {
        //serial.write("pml4 idx {}\r\n", .{pml4_idx});
        return sRet;
    } else {
        table = @ptrFromInt(pml4.?[pml4_idx] & ~@as(u64, 0x3));
    }

    if (table[pdpt_idx] == 0) {
        sRet = mem.alloc_pages(1, @ptrCast(&page));
        @memset(page[0..512], 0);
        table[pdpt_idx] = @intFromPtr(page) | 0x3;
        table = page;
    } else {
        table = @ptrFromInt(table[pdpt_idx] & ~@as(u64, 0x3));
    }

    if (table[pdt_idx] == 0) {
        sRet = mem.alloc_pages(1, @ptrCast(&page));
        @memset(page[0..512], 0);
        table[pdt_idx] = @intFromPtr(page) | 0x3;
        table = page;
    } else {
        table = @ptrFromInt(table[pdt_idx] & ~@as(u64, 0x3));
    }

    table[pt_idx] = addr | 0x3;
    return status.success;
}

pub fn IdentityMapImage(addr: []u8) !void {
    var page: []align(4096) Page = undefined;
    var table: [*]align(4096) u64 = undefined;
    const num_pages = addr.len >> 12;
    const image_base = @intFromPtr(addr.ptr) & ~@as(u64, 0xFFF);

    if (pml4 == null) {
        serial.write("Base page table pml4 not allocated\r\n", .{});
        return Error.Unexpected;
    }

    const pml4_idx = (image_base >> 39) & maxInt(u9);
    const pdpt_idx = (image_base >> 30) & maxInt(u9);
    const pdt_idx = (image_base >> 21) & maxInt(u9);
    const pt_idx = (image_base >> 12) & maxInt(u9);
    ////serial.write("Identity mapping {*} size:0x{x} \r\npml4e_idx {} pdpt_idx {} pdt_idx {} pt_idx {}\r\n", .{ addr, size, pml4_idx, pdpt_idx, pdt_idx, pt_idx });

    // Dont use address space above 512GB
    if (pml4_idx != 0) {
        return Error.Unexpected;
    } else {
        table = @ptrFromInt(pml4.?[pml4_idx] & ~@as(u64, 0x3));
    }

    if (table[pdpt_idx] == 0) {
        page = try mem.alloc_pages(1);
        @memset(page[0][0..], 0);
        table[pdpt_idx] = @intFromPtr(page.ptr) | 0x3;
        table = @ptrCast(page.ptr);
    } else {
        table = @ptrFromInt(table[pdpt_idx] & ~@as(u64, 0x3));
    }

    if (table[pdt_idx] == 0) {
        page = try mem.alloc_pages(1);
        @memset(page[0][0..], 0);
        table[pdt_idx] = @intFromPtr(page.ptr) | 0x3;
        table = @ptrCast(page.ptr);
    } else {
        table = @ptrFromInt(table[pdt_idx] & ~@as(u64, 0x3));
    }

    for (0..num_pages) |i| {
        if (pt_idx + i >= 512) return Error.Aborted;
        table[pt_idx + i] = image_base + (i << 12) | 0x3;
    }
}

pub fn MapUsableMemory(mmap: MemoryMapSlice, kStart: usize) !void {
    var itr = mmap.iterator();
    while (true) {
        const desc = itr.next();
        if (desc == null) break;
        totalMemPages += desc.?.number_of_pages;
    }
    serial.write("Total pages to map 0x{x}\n", .{totalMemPages});

    const num_pt = (totalMemPages >> 9) + 1;
    const num_pdt = (num_pt >> 9) + 1;
    const num_pdpt = (num_pdt >> 9) + 1;
    const num_pml4 = (num_pdpt >> 9) + 1;
    const num_kpdt = 1;
    const num_kpt = 1;
    const table_entries = 512;

    serial.write("Mapping pages: 0x{x}. PML4 = {} PDPT {} PDT {} PT {} \n", .{ totalMemPages, num_pml4, num_pdpt, num_pdt, num_pt });
    const total_pages = num_pt + num_pdt + num_pdpt + num_pml4 + num_kpdt + num_kpt;

    const pMem = try mem.alloc_pages(total_pages);
    @memset(pMem[0..total_pages], [_]u8{0} ** 4096);
    serial.write("Allocated pml4 at {*}\r\n", .{pMem.ptr});
    pml4 = @ptrCast(pMem.ptr);

    var start: usize = 0;
    var end: usize = start + (num_pml4 * table_entries);
    var PML4 = pml4.?[start..table_entries];

    start = end;
    end = start + (num_pdpt * table_entries);
    var PDPT = pml4.?[start..end];

    start = end;
    end = start + (num_pdt * table_entries);
    var PDT = pml4.?[start..end];

    start = end;
    end = start + (num_kpdt * table_entries);
    var KPDT = pml4.?[start..end];

    start = end;
    end = start + (num_pt * table_entries);
    var PT = pml4.?[start..end];

    const kEnd = kStart + (num_kpt * (2 << 20));
    serial.write("Kernel range 0x{x} - 0x{x}\r\n", .{ kStart, kEnd });

    var addr: u64 = 0x3;
    for (0..num_pt << 9) |i| {
        if (addr >= kStart and addr < kEnd) continue;
        PT[i] = addr;
        addr += 0x1000;
    }

    start = end;
    end = start + (num_kpt * table_entries);
    var KPT = pml4.?[start..end];
    addr = kStart | 0x3;
    for (0..num_kpt << 9) |i| {
        KPT[i] = addr;
        addr += 0x1000;
    }

    var idx: usize = (kMemAddr >> 39) & maxInt(u9);
    assert(idx == 0);
    PML4[idx] = @intFromPtr(PDPT.ptr) | 0x3;

    idx = (kMemAddr >> 30) & maxInt(u9);
    assert(idx < 512);
    for (0..num_pdt) |i| {
        PDPT[i] = @intFromPtr(&PDT[i << 9]) | 0x3; //1GB
        //serial.write("PDPT {*}[{}][{}] -> 0x{x}\n", .{ pPDPT, i, pdpt_idx + j, pPDPT[i][pdpt_idx + j] });
    }

    idx = (kMemAddr >> 21) & maxInt(u9);
    assert(idx < 512);
    for (0..num_pt) |i| {
        PDT[i] = @intFromPtr(&PT[i << 9]) | 0x3; //1GB
        //serial.write("PDPT {*}[{}][{}] -> 0x{x}\n", .{ pPDPT, i, pdpt_idx + j, pPDPT[i][pdpt_idx + j] });
    }

    idx = (kCompAddr >> 30) & maxInt(u9);
    assert(idx < 512);
    PDPT[idx] = @intFromPtr(KPDT.ptr) | 0x3;

    idx = (kCompAddr >> 21) & maxInt(u9);
    KPDT[idx] = @intFromPtr(KPT.ptr) | 0x3;
}
