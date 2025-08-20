const serial = @import("serial.zig");
const maxInt = @import("std").math.maxInt;
const base_addr = 0x4040000000;
pub var g_pml4: ?*[512]usize = undefined;

pub fn init_kernel_pages(pml4_base: u64, num_pages: u64) void {
    const num_pt = (num_pages >> 9) + 1;
    const num_pdt = (num_pt >> 9) + 1;
    const num_pdpt = (num_pdt >> 9) + 1;
    const num_pml4 = (num_pdpt >> 9) + 1;
    const table_size = @sizeOf([512]u64);
    const table_entries = 512;

    serial.write("pml4: 0x{x} pdpt: 0x{x} pdt: 0x{x} pt: 0x{x} pml4_base: 0x{x}\n", .{ num_pml4, num_pdpt, num_pdt, num_pt, pml4_base });

    g_pml4 = @ptrFromInt(pml4_base);

    const pml4: [*][512]usize = @ptrFromInt(pml4_base);
    const pdpt: [*][512]usize = @ptrFromInt(@intFromPtr(pml4) + (num_pml4 * table_size));
    const pdt: [*][512]usize = @ptrFromInt(@intFromPtr(pdpt) + (num_pml4 * table_size));
    const pt: [*][512]usize = @ptrFromInt(@intFromPtr(pdt) + (num_pml4 * table_size));

    //pml4[0][0] = @intFromPtr(pdpt) | 0x3;
    //pdpt[0][0] = @intFromPtr(pdt) | 0x3;
    //pdt[0][0] = @intFromPtr(pt) | 0x3;
    serial.write("pml4: {*} pdpt: {*} pdt: {*} pt: {*}\n", .{ &pml4[0][0], &pdpt[0][0], &pdt[0][0], &pt[0][0] });

    var addr: usize = base_addr;
    for (0..num_pt) |pt_idx| {
        for (0..table_entries) |pte_idx| {
            pt[pt_idx][pte_idx] = addr | 0x3;
            addr += 0x1000;
        }
        serial.write("Writing PT: 0x{x}\n", .{pt_idx});
    }
}
