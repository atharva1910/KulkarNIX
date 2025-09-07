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
const MemoryType = uefi.tables.MemoryType;
const Error = uefi.Error;

pub const kCompAddr = 0x4000000000;
pub const kMemAddr = 0x4040000000;
pub var pml4: ?[*]u64 = undefined;
pub var PageTables: [][512]u64 = undefined;
pub var totalMemPages: usize = undefined;

pub const PageTableMgr = struct {
    PageTables: [][512]u64,
    NumPDPT: usize,
    NumPDT: usize,
    NumPT: usize,
    TotalPages: usize,
    PageTablePages: usize,
};

fn DumpTable(table: [*]u64, lvl: u16) void {
    var tname: []const u8 = undefined;
    switch (lvl) {
        0 => tname = "PML4",
        1 => tname = "PDPT",
        //2 => tname = "PDT",
        //3 => tname = "PT",
        else => return,
    }

    if (lvl < 2) {
        for (0..512) |i| {
            if (table[i] == 0) continue;
            serial.write("{s}[{*}] -> [{}] 0x{x}\r\n", .{ tname, table, i, table[i] });
            DumpTable(@ptrFromInt(table[i] & ~@as(u64, 0x3)), lvl + 1);
        }
    } else {
        serial.write("{s}[{*}] -> [{}] 0x{x}\r\n", .{ tname, table, 0, table[0] });
        serial.write("{s}[{*}] -> [{}] 0x{x}\r\n", .{ tname, table, 511, table[511] });
    }
}

pub fn DumpPageMap() void {
    if (pml4 == null) {
        serial.write("Base page table pml4 not allocated\r\n", .{});
        return;
    }
    DumpTable(pml4.?, 0);
}

pub fn IdentityMapImage(addr: []u8) !void {
    var table: []align(4096) u64 = undefined;
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
    //serial.write("Identity mapping {*} size:0x{x} \r\npml4e_idx {} pdpt_idx {} pdt_idx {} pt_idx {}\r\n", .{ addr, addr.len, pml4_idx, pdpt_idx, pdt_idx, pt_idx });

    // Dont use address space above 512GB
    if (pml4_idx != 0) {
        return Error.Unexpected;
    } else {
        const page: [*]align(4096) u64 = @ptrFromInt(PageTables[0][pml4_idx] & ~@as(u64, 0x3));
        table = page[0..512];
    }

    if (table[pdpt_idx] == 0) {
        const page: []align(4096) u64 = @ptrCast(try mem.alloc_pages(1));
        assert(page.len == 512);
        @memset(page, 0);
        table[pdpt_idx] = @intFromPtr(page.ptr) | 0x3;
        table = page;
    } else {
        const page: [*]align(4096) u64 = @ptrFromInt(table[pdpt_idx] & ~@as(u64, 0x3));
        table = page[0..512];
    }

    if (table[pdt_idx] == 0) {
        const page: []align(4096) u64 = @ptrCast(try mem.alloc_pages(1));
        assert(page.len == 512);
        @memset(page, 0);
        table[pdt_idx] = @intFromPtr(page.ptr) | 0x3;
        table = page;
    } else {
        const page: [*]align(4096) u64 = @ptrFromInt(table[pdt_idx] & ~@as(u64, 0x3));
        table = page[0..512];
    }

    for (0..num_pages) |i| {
        if (pt_idx + i >= 512) return Error.Aborted;
        table[pt_idx + i] = image_base + (i << 12) | 0x3;
    }
}

pub fn MapUsableMemory(mmap: MemoryMapSlice, kStart: usize) !PageTableMgr {
    var ret: PageTableMgr = undefined;
    var itr = mmap.iterator();
    while (itr.next()) |desc| {
        if (desc.type == MemoryType.boot_services_code or
            desc.type == MemoryType.boot_services_data or
            desc.type == MemoryType.conventional_memory)
        {
            totalMemPages = (desc.physical_start + (desc.number_of_pages << 12)) >> 12;
        }
    }

    const num_pt = (totalMemPages >> 9) + 1;
    const num_pdt = (num_pt >> 9) + 1;
    const num_pdpt = (num_pdt >> 9) + 1;
    const num_pml4 = (num_pdpt >> 9) + 1;
    const num_kpdt = 1;
    const num_kpt = 1;
    const table_entries = 512;

    serial.write("Mapping pages: 0x{x}. PML4 = {} PDPT {} PDT {} PT {} \n", .{ totalMemPages, num_pml4, num_pdpt, num_pdt, num_pt });
    const total_pages = num_pt + num_pdt + num_pdpt + num_pml4 + num_kpdt + num_kpt;
    ret.NumPDPT = num_pdpt;
    ret.NumPDT = num_pdt;
    ret.NumPT = num_pt;
    ret.TotalPages = totalMemPages;
    ret.PageTablePages = total_pages;

    PageTables = @ptrCast(try mem.alloc_pages(total_pages));
    serial.write("Page Tables Start: {*} End: 0x{x}\n", .{ &PageTables[0], @intFromPtr(&PageTables[0]) + (total_pages << 12) });
    @memset(PageTables[0..total_pages], [_]u64{0} ** 512);
    pml4 = @ptrCast(@alignCast(PageTables.ptr));

    var start: usize = 0;
    var end: usize = num_pml4;
    var PML4 = PageTables[start..end];
    //serial.write("PML4 = pml4.?[0x{x} - 0x{x}] = 0x{x}\r\n", .{ start, end, PML4.len });
    assert(PML4.len == num_pml4);

    start = end;
    end = start + num_pdpt;
    var PDPT = PageTables[start..end];
    assert(PDPT.len == num_pdpt);
    //serial.write("PDPT = pml4.?[0x{x} - 0x{x}] = {*}\r\n", .{ start, end, PDPT });

    start = end;
    end = start + num_pdt;
    var PDT = PageTables[start..end];
    assert(PDT.len == num_pdt);
    //serial.write("PDT = pml4.?[0x{x} - 0x{x}] = {*}\r\n", .{ start, end, PDT });

    start = end;
    end = start + num_kpdt;
    var KPDT = PageTables[start..end];
    assert(KPDT.len == num_kpdt);
    //serial.write("KPDT = pml4.?[0x{x} - 0x{x}] = {*}\r\n", .{ start, end, KPDT });

    start = end;
    end = start + num_pt;
    var PT = PageTables[start..end];
    assert(PT.len == num_pt);
    //serial.write("PT = pml4.?[0x{x} - 0x{x}] = {*}\r\n", .{ start, end, PT });

    var addr: u64 = 0x3;
    for (0..num_pt) |i| {
        for (0..table_entries) |j| {
            PT[i][j] = addr;
            addr += 0x1000;
        }
    }

    start = end;
    end = start + num_kpt;
    var KPT = PageTables[start..end];
    assert(KPT.len == num_kpt);

    addr = kStart | 0x3;
    for (0..num_kpt) |i| {
        for (0..table_entries) |j| {
            KPT[i][j] = addr;
            addr += 0x1000;
        }
    }

    var idx: usize = (kMemAddr >> 39) & maxInt(u9);
    assert(idx == 0);
    PML4[0][idx] = @intFromPtr(PDPT.ptr) | 0x3;

    idx = (kMemAddr >> 30) & maxInt(u9);
    assert(idx == 257);
    for (0..num_pdpt) |i| {
        const pdpt = &PDPT[i];
        for (0..table_entries) |j| {
            const pdt_idx = (i << 9) + j;
            if (pdt_idx == num_pdt) break;
            pdpt[idx + j] = @intFromPtr(&PDT[pdt_idx]) | 0x3; //1GB
        }
        idx = 0;
    }

    idx = (kMemAddr >> 21) & maxInt(u9);
    assert(idx == 0);
    for (0..num_pdt) |i| {
        const pdt = &PDT[i];
        for (0..table_entries) |j| {
            const pt_idx = (i << 9) + j;
            if (pt_idx == num_pt) break;
            pdt[j] = @intFromPtr(&PT[pt_idx]) | 0x3;
        }
    }

    idx = (kCompAddr >> 30) & maxInt(u9);
    assert(idx == 256);
    PDPT[0][idx] = @intFromPtr(KPDT.ptr) | 0x3;

    idx = (kCompAddr >> 21) & maxInt(u9);
    assert(idx == 0);
    KPDT[0][idx] = @intFromPtr(KPT.ptr) | 0x3;

    return ret;
}

pub fn isPagePresent(addr: usize) bool {
    if (pml4 == null) return false;

    const pml4_idx = (addr >> 39) & maxInt(u9);
    const pdpt_idx = (addr >> 30) & maxInt(u9);
    const pdt_idx = (addr >> 21) & maxInt(u9);
    const pt_idx = (addr >> 12) & maxInt(u9);

    if (pml4.?[pml4_idx] == 0) {
        serial.write("No PML4 entry\n", .{});
        return false;
    }

    const PDPT: [*]u64 = @ptrFromInt(pml4.?[pml4_idx] & ~@as(u64, 0x3));
    if (PDPT[pdpt_idx] == 0) {
        serial.write("No PDPT entry\n", .{});
        return false;
    }

    const PDT: [*]u64 = @ptrFromInt(PDPT[pdpt_idx] & ~@as(u64, 0x3));
    if (PDT[pdt_idx] == 0) {
        serial.write("No PDT entry\n", .{});
        return false;
    }

    const PT: [*]u64 = @ptrFromInt(PDT[pdt_idx] & ~@as(u64, 0x3));
    if (PT[pt_idx] == 0) {
        serial.write("No PT entry Index: PML4 {} PDPT:{} PDT:{} PT:{}\n", .{ pml4_idx, pdpt_idx, pdt_idx, pt_idx });
        return false;
    }

    return true;
}
