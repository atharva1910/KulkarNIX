const uefi = @import("std").os.uefi;
const status = uefi.Status;
const elf = @import("std").elf;
const main = @import("main.zig");
const MemoryDescriptor = @import("std").os.uefi.tables.MemoryDescriptor;
const serial = @import("serial.zig");
const paging = @import("paging.zig");

pub fn alloc_pages(pages: usize, buf: *[*]align(4096) u8) uefi.Status {
    return main.boot_services.allocatePages(uefi.tables.AllocateType.allocate_any_pages, uefi.tables.MemoryType.boot_services_data, pages, buf);
}

pub fn free(buf: [*]align(8) u8) void {
    _ = main.boot_services.freePool(buf);
}

pub fn alloc(size: usize, buf: *[*]align(8) u8) uefi.Status {
    return main.boot_services.allocatePool(uefi.tables.MemoryType.boot_services_data, size, buf);
}

pub fn get_memory_map(size: *usize, buf: *[*]align(4096) MemoryDescriptor, key: *usize, descSize: *usize, descVer: *u32) uefi.Status {
    var sRet = main.boot_services.getMemoryMap(size, @ptrCast(buf.*), key, descSize, descVer);
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

    sRet = paging.identity_map_pages(@ptrCast(buf.*), num_pages);
    if (sRet != status.success) {
        serial.write("Failed to identity map memory map\r\n", .{});
        return sRet;
    }

    sRet = main.boot_services.getMemoryMap(size, buf.*, key, descSize, descVer);
    if (sRet != status.success) {
        serial.write("getMemoryMap failed\r\n", .{});
        return sRet;
    }

    //serial.write("Memory map stored in page {*} num_pages: {}, size: {}\r\n", .{ buf.*, num_pages, size.* });
    return sRet;
}
