const uefi = @import("std").os.uefi;
const serial = @import("serial.zig");
const assert = @import("std").debug.assert;
const status = uefi.Status;
const mem = @import("memory.zig");
const screen = @import("screen.zig");
const maxInt = @import("std").math.maxInt;
pub const kCompAddr = 0x4000000000;
pub const kMemAddr = 0x4040000000;
pub var pml4: ?[*]u64 = undefined;
pub var totalMemPages: usize = undefined;

const PageType = enum(u2) { PML4, PDPT, PDT, PT };

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

pub fn IdentityMapImage(addr: [*]u8, size: usize) uefi.Status {
    var sRet: uefi.Status = undefined;
    var page: [*]align(4096) u64 = undefined;
    var table: [*]align(4096) u64 = undefined;
    const num_pages = size >> 12;
    const image_base = @intFromPtr(addr) & ~@as(u64, 0xFFF);

    if (pml4 == null) {
        //serial.write("Base page table pml4 not allocated\r\n", .{});
        return sRet;
    }

    const pml4_idx = (image_base >> 39) & maxInt(u9);
    const pdpt_idx = (image_base >> 30) & maxInt(u9);
    const pdt_idx = (image_base >> 21) & maxInt(u9);
    const pt_idx = (image_base >> 12) & maxInt(u9);
    ////serial.write("Identity mapping {*} size:0x{x} \r\npml4e_idx {} pdpt_idx {} pdt_idx {} pt_idx {}\r\n", .{ addr, size, pml4_idx, pdpt_idx, pdt_idx, pt_idx });

    // Dont use address space above 512GB
    if (pml4_idx != 0) {
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

    for (0..num_pages) |i| {
        if (pt_idx + i >= 512) return status.load_error;
        table[pt_idx + i] = image_base + (i << 12) | 0x3;
    }

    //DumpPageMap();
    return sRet;
}

pub fn MapUsableMemory(cmap: [*]mem.clubbed_entry, num_entries: usize, kAddr: [*]u8) uefi.Status {
    var mtotal_pages: u64 = 0;
    for (0..num_entries) |idx| {
        mtotal_pages += cmap[idx].num_pages;
    }
    totalMemPages = mtotal_pages;

    const num_pt = (mtotal_pages >> 9) + 1;
    const num_pdt = (num_pt >> 9) + 1;
    const num_pdpt = (num_pdt >> 9) + 1;
    const num_pml4 = (num_pdpt >> 9) + 1;
    const num_kpdt = 1;
    const num_kpt = 1;
    const table_entries = 512;
    const table_size: u64 = @sizeOf([512]u64);

    serial.write("Mapping pages: 0x{x}. PML4 = {} PDPT {} PDT {} PT {} \n", .{ mtotal_pages, num_pml4, num_pdpt, num_pdt, num_pt });
    const total_pages = num_pt + num_pdt + num_pdpt + num_pml4 + num_kpdt + num_kpt;

    const sRet = mem.alloc_pages(total_pages, @ptrCast(&pml4));
    if (sRet != status.success) {
        //serial.write("Failed to allocate memory for mapping memory\r\n", .{});
        return sRet;
    } else {
        serial.write("Allocated pml4 at {*}\r\n", .{pml4.?});
        @memset(pml4.?[0 .. table_entries * total_pages], 0);
    }

    var pPML4: []u64 = pml4.?[0..512];
    var pPDPT: [*][512]u64 = @ptrFromInt(@intFromPtr(&pPML4[0]) + (num_pml4 * table_size));
    var pKPDT: [*][512]u64 = @ptrFromInt(@intFromPtr(&pPDPT[0][0]) + (num_pdpt * table_size));
    var pPDT: [*][512]u64 = @ptrFromInt(@intFromPtr(&pKPDT[0][0]) + (num_kpdt * table_size));
    var pKPT: [*][512]u64 = @ptrFromInt(@intFromPtr(&pPDT[0][0]) + (num_pdt * table_size));
    var pPT: [*][512]u64 = @ptrFromInt(@intFromPtr(&pKPT[0][0]) + (num_kpt * table_size));

    const kStart = @intFromPtr(kAddr);
    const kEnd = kStart + (num_kpt * (2 << 20));
    serial.write("Kernel range 0x{x} - 0x{x}\r\n", .{ kStart, kEnd });

    // Fill Page Tables
    var addr: u64 = 0x0;
    for (0..num_pt) |i| {
        for (0..table_entries) |j| {
            defer addr += 0x1000;
            if (addr >= kStart and addr < kEnd) continue;
            pPT[i][j] = addr | 0x3; //2MB
        }
        //serial.write("pPT {*}[{}] -> 0x{x} - 0x{x}\n", .{ &pPT[i][0], i, pPT[i][0], pPT[i][511] });
    }

    addr = @intFromPtr(kAddr) | 0x3;
    for (0..table_entries) |i| {
        pKPT[0][i] = addr;
        addr += 0x1000;
    }
    //serial.write("pKPT {*}[{}] -> 0x{x} - 0x{x}\n", .{ pKPT, 0, pKPT[0][0], pKPT[0][511] });

    // Fill Paging Structures
    const pml4_idx = (kMemAddr >> 39) & maxInt(u9);
    var pdpt_idx: u32 = (kMemAddr >> 30) & maxInt(u9);
    var pdt_idx: u32 = (kMemAddr >> 21) & maxInt(u9);

    //serial.write("Index: PML4 {} PDPT {} PDT {} \n", .{ pml4_idx, pdpt_idx, pdt_idx });
    assert(pml4_idx == 0);
    assert(pdpt_idx < 512);
    assert(pdt_idx < 512);

    pPML4[pml4_idx] = @intFromPtr(&pPDPT[0][0]) | 0x3; //512GB
    //serial.write("PML4 {*}[0x{x}][0] -> 0x{x}\n", .{ pPML4, pml4_idx, pPML4[pml4_idx] });

    for (0..num_pdpt) |i| {
        for (0..table_entries) |j| {
            const offset = i * 512 + j;
            if (offset == num_pdt) break;
            pPDPT[i][pdpt_idx + j] = @intFromPtr(&pPDT[offset][0]) | 0x3; //1GB
            //serial.write("PDPT {*}[{}][{}] -> 0x{x}\n", .{ pPDPT, i, pdpt_idx + j, pPDPT[i][pdpt_idx + j] });
        }
    }

    for (0..num_pdt) |i| {
        for (0..table_entries) |j| {
            const offset = i * 512 + j;
            if (offset == num_pt) break;
            pPDT[i][pdt_idx + j] = @intFromPtr(&pPT[offset][0]) | 0x3; //2MB
        }
        //serial.write("pPDT {*}[{}] -> 0x{x} - 0x{x}\n", .{ &pPDT[i][0], i, pPDT[i][0], pPDT[i][511] });
    }

    pdpt_idx = (kCompAddr >> 30) & maxInt(u9);
    pdt_idx = (kCompAddr >> 21) & maxInt(u9);
    pPDPT[0][pdpt_idx] = @intFromPtr(&pKPDT[0][0]) | 0x3; //1GB
    //serial.write("KPDPT {*}[0x{x}][0] -> 0x{x}\n", .{ pPDPT, pdpt_idx, pPDPT[0][pdpt_idx] });
    pKPDT[0][pdt_idx] = @intFromPtr(&pKPT[0][0]) | 0x3; //2MB
    //serial.write("KPDT {*}[0x{x}][0] -> 0x{x}\n", .{ pKPDT, pdt_idx, pKPDT[0][pdt_idx] });

    return sRet;
}
