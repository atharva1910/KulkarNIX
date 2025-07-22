const uefi = @import("std").os.uefi;
const elf = @import("std").elf;
const status = uefi.Status;
const L = @import("std").unicode.utf8ToUtf16LeStringLiteral;
const screen = @import("screen.zig");
const mem = @import("memory.zig");
const paging = @import("paging.zig");
const serial = @import("serial.zig");
const MemoryDescriptor = @import("std").os.uefi.tables.MemoryDescriptor;
const kCompAddr = 0x4000000000;
pub var boot_services: *uefi.tables.BootServices = undefined;
//pub const pageRoot: paging.pml4 = undefined;
var loaded_img: *uefi.protocol.LoadedImage = undefined;

fn stall(ms: u64) void {
    _ = uefi.system_table.boot_services.?.stall(ms);
}

fn ReadKernel(pages: [*]u8) ?u64 {
    var sRet: uefi.Status = undefined;
    var retAddr: ?u64 = null;

    sRet = boot_services.handleProtocol(uefi.handle, &uefi.protocol.LoadedImage.guid, @ptrCast(&loaded_img));
    if (status.success != sRet) {
        serial.write("LoadedImageProtocol failed\r\n", .{});
        return retAddr;
    }

    var sfs: *uefi.protocol.SimpleFileSystem = undefined;
    sRet = boot_services.handleProtocol(loaded_img.device_handle.?, &uefi.protocol.SimpleFileSystem.guid, @ptrCast(&sfs));
    if (status.success != sRet) {
        serial.write("Failed to get handle to SFS\r\n", .{});
        return retAddr;
    }

    var fileProt: *uefi.protocol.File = undefined;
    sRet = sfs.openVolume(@ptrCast(&fileProt));
    if (status.success != sRet) {
        serial.write("Failed to open volume\r\n", .{});
        return retAddr;
    }

    var kHandle: *const uefi.protocol.File = undefined;
    sRet = fileProt.open(&kHandle, L("\\Kernel.elf"), 1, 1);
    if (status.success != sRet) {
        serial.write("Failed to open Kernel.elf\r\n", .{});
        return retAddr;
    }
    defer _ = fileProt.close();

    var elf_hdr: elf.Ehdr = undefined;
    var hdr_size: usize = @sizeOf(elf.Ehdr);
    sRet = kHandle.read(&hdr_size, @ptrCast(&elf_hdr));
    if (status.success != sRet) {
        serial.write("Failed to read Kernel.elf\r\n", .{});
        return retAddr;
    }

    sRet = kHandle.setPosition(elf_hdr.e_phoff);
    if (status.success != sRet) return retAddr;

    var phdr_size: usize = elf_hdr.e_phnum * elf_hdr.e_phentsize;
    var p_phdr: [*]align(8) elf.Elf64_Phdr = undefined;

    sRet = mem.alloc(phdr_size, @ptrCast(&p_phdr));
    if (status.success != sRet) return retAddr;
    defer mem.free(@ptrCast(p_phdr));

    sRet = kHandle.read(&phdr_size, @ptrCast(p_phdr));
    if (status.success != sRet) return retAddr;

    retAddr = elf_hdr.e_entry;
    for (0..elf_hdr.e_phnum) |i| {
        if (p_phdr[i].p_type != elf.PT_LOAD)
            continue;

        const buf_idx = p_phdr[i].p_paddr - kCompAddr;
        var buf_size = p_phdr[i].p_filesz;
        sRet = kHandle.setPosition(p_phdr[i].p_offset);

        if ((status.success != sRet) or
            (status.success != kHandle.read(&buf_size, pages[buf_idx..])) or
            (buf_size < p_phdr[i].p_filesz))
        {
            break;
        } else {
            serial.write("phdr[{}] addr {s} size 0x{x}\r\n", .{ i, &pages[buf_idx], p_phdr[i].p_filesz });
        }
    }

    serial.write("Kernel loaded into memory at {*} entry 0x{x}!\r\n", .{ pages, retAddr.? });
    return retAddr;
}

pub fn main() void {
    var sRet: uefi.Status = undefined;

    boot_services = uefi.system_table.boot_services.?;
    screen.init();
    screen.clrscr();

    if (serial.init() == false) {
        serial.write("serial port init - failed", .{});
        stall(0xFFFFFFFFFFFFFFFF);
        return;
    }

    var kernel: [*]align(4096) u8 = undefined;
    sRet = mem.alloc_pages(512, &kernel);
    if (status.success != sRet) return;

    const entry = ReadKernel(kernel);
    if (entry == null) {
        serial.write("Entry null?\r\n", .{});
        stall(0xFFFFFFFFFFFFFFFF);
    }

    sRet = paging.SetupPaging(kernel);
    if (status.success != sRet) {
        serial.write("Failed to setup Paging\r\n", .{});
        stall(0xFFFFFFFFFFFFFFFF);
    }

    sRet = paging.IdentityMapImage(loaded_img.image_base, loaded_img.image_size);
    if (status.success != sRet) {
        serial.write("Failed to setup Identity Map Image\r\n", .{});
        stall(0xFFFFFFFFFFFFFFFF);
    }

    if (status.success != boot_services.setWatchdogTimer(0, 0, 0, null)) {
        serial.write("Failed to disable watchdog timer\r\n", .{});
        stall(0xFFFFFFFFFFFFFFFF);
    }

    var size: usize = 0;
    var mmap: [*]MemoryDescriptor = undefined;
    var key: usize = undefined;
    var descSize: usize = undefined;
    var descVer: u32 = undefined;
    if (status.success == mem.get_memory_map(&size, &mmap, &key, &descSize, &descVer)) {
        if (status.success != boot_services.exitBootServices(uefi.handle, key)) {
            serial.write("Exit boot services failed\r\n", .{});
            stall(0xFFFFFFFFFFFFFFFF);
        }
    }

    if (paging.pml4 == null) {
        serial.write("NULL PML4\r\n", .{});
        stall(0xFFFFFFFFFFFFFFFF);
    }

    serial.write("Replacing Page Tables {*} and jumping to 0x{x}\r\n", .{ paging.pml4.?, entry.? });
    asm volatile (
        \\mov %[pml4], %%rax
        \\mov %%rax, %%cr3
        \\jmp *%[entry]
        :
        : [pml4] "r" (paging.pml4.?),
          [entry] "r" (entry.?),
        : "rax"
    );
}
