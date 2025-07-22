const uefi = @import("std").os.uefi;
const serial = @import("serial.zig");
const assert = @import("std").debug.assert;
const status = uefi.Status;
const mem = @import("memory.zig");
const screen = @import("screen.zig");
const maxInt = @import("std").math.maxInt;
const kCompAddr = 0x4000000000;

pub var pml4: ?[*]u64 = undefined;

const PageType = enum(u2) { PML4, PDPT, PDT, PT };

pub fn SetupPaging(kAddr: [*]u8) uefi.Status {
    var sRet: uefi.Status = undefined;
    var pdpt: [*]u64 = undefined;
    var pdt: [*]u64 = undefined;
    var pt: [*]u64 = undefined;

    // 1 of each to identity map 2MB Kernel
    sRet = mem.alloc_pages(4, @ptrCast(&pml4));
    if (status.success != sRet) {
        serial.write("Failed to allocate memory for kernel pages\r\n", .{});
        return sRet;
    }
    @memset(pml4.?[0 .. 512 * 4], 0);

    pdpt = @ptrCast(&pml4.?[512]);
    pdt = @ptrCast(&pml4.?[512 * 2]);
    pt = @ptrCast(&pml4.?[512 * 3]);

    const pml4_idx = (kCompAddr >> 39) & maxInt(u9);
    const pdpt_idx = (kCompAddr >> 30) & maxInt(u9);
    const pdt_idx = (kCompAddr >> 21) & maxInt(u9);

    pml4.?[pml4_idx] = @intFromPtr(pdpt) | 0x3;
    pdpt[pdpt_idx] = @intFromPtr(pdt) | 0x3;
    pdt[pdt_idx] = @intFromPtr(pt) | 0x3;

    for (0..512) |i| {
        pt[i] = @intFromPtr(kAddr) + (i << 12) | 0x3;
    }

    serial.write("Mapped Kernel from {*} to 0x{x}. PML4 Base {*}\r\n", .{ kAddr, kCompAddr, pml4.? });
    return sRet;
}

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

pub fn IdentityMapImage(addr: [*]u8, size: usize) uefi.Status {
    var sRet: uefi.Status = undefined;
    var page: [*]align(4096) u64 = undefined;
    var table: [*]align(4096) u64 = undefined;
    const num_pages = size >> 12;
    const image_base = @intFromPtr(addr) & ~@as(u64, 0xFFF);

    if (pml4 == null) {
        serial.write("Base page table pml4 not allocated\r\n", .{});
        return sRet;
    }

    const pml4_idx = (image_base >> 39) & maxInt(u9);
    const pdpt_idx = (image_base >> 30) & maxInt(u9);
    const pdt_idx = (image_base >> 21) & maxInt(u9);
    const pt_idx = (image_base >> 12) & maxInt(u9);
    //serial.write("Identity mapping {*} size:0x{x} \r\npml4e_idx {} pdpt_idx {} pdt_idx {} pt_idx {}\r\n", .{ addr, size, pml4_idx, pdpt_idx, pdt_idx, pt_idx });

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
