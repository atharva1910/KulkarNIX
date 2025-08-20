const uefi = @import("std").os.uefi;
const elf = @import("std").elf;
const status = uefi.Status;
const L = @import("std").unicode.utf8ToUtf16LeStringLiteral;
const screen = @import("screen.zig");
const mem = @import("memory.zig");
const paging = @import("paging.zig");
const serial = @import("serial.zig");
const kargs = @import("kargs.zig").kargs;
const MemoryDescriptor = @import("std").os.uefi.tables.MemoryDescriptor;
const kCompAddr = 0x4000000000;

pub var boot_services: *uefi.tables.BootServices = undefined;
var loaded_img: *uefi.protocol.LoadedImage = undefined;
var gop: *uefi.protocol.GraphicsOutput = undefined;

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
            //} else {
            //  serial.write("phdr[{}] addr {s} size 0x{x}\r\n", .{ i, &pages[buf_idx], p_phdr[i].p_filesz });
        }
    }

    return retAddr;
}

fn init_gop() uefi.Status {
    var sRet: uefi.Status = undefined;

    sRet = boot_services.locateProtocol(&uefi.protocol.GraphicsOutput.guid, null, @ptrCast(&gop));
    if (sRet != status.success) {
        serial.write("Failed to load Graphics Output protocol {}\r\n", .{sRet});
        return sRet;
    }

    var smode: u32 = undefined;
    for (0..gop.mode.max_mode) |i| {
        var info_size: usize = undefined;
        var info: *uefi.protocol.GraphicsOutput.Mode.Info = undefined;
        const mode: u32 = @intCast(i);
        sRet = gop.queryMode(mode, &info_size, &info);
        if (sRet != status.success) {
            serial.write("Failed to query gop mode {} status: {}\r\n", .{ i, sRet });
            continue;
        }

        if (info.horizontal_resolution == 800 and info.vertical_resolution == 600) {
            smode = mode;
            serial.write("Setting GOP mode {} {}x{}\r\n", .{ i, info.horizontal_resolution, info.vertical_resolution });
            break;
        }
    }

    sRet = gop.setMode(smode);
    if (sRet != status.success) {
        serial.write("Failed to set GOP mode{}\r\n", .{smode});
    }

    return sRet;
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

    var argsPage: *align(4096) kargs = undefined;
    sRet = mem.alloc_pages(1, @ptrCast(&argsPage));
    if (status.success != sRet) return;

    var kernel: [*]align(4096) u8 = undefined;
    sRet = mem.alloc_pages(512, &kernel);
    if (status.success != sRet) return;

    const entry = ReadKernel(kernel);
    if (entry == null) {
        serial.write("Entry null?\r\n", .{});
        stall(0xFFFFFFFFFFFFFFFF);
    }

    if (status.success != init_gop()) {
        serial.write("Failed to init gop\r\n", .{});
        stall(0xFFFFFFFFFFFFFFFF);
    }

    if (status.success != boot_services.setWatchdogTimer(0, 0, 0, null)) {
        serial.write("Failed to disable watchdog timer\r\n", .{});
        stall(0xFFFFFFFFFFFFFFFF);
    }

    var mmapSize: usize = 0;
    var mmap: [*]align(4096) MemoryDescriptor = undefined;
    var key: usize = undefined;
    var descSize: usize = undefined;
    var descVer: u32 = undefined;
    if (status.success != mem.GetMemoryMap(&mmapSize, &mmap, &key, &descSize, &descVer)) {
        serial.write("Failed to get memory_map\r\n", .{});
        stall(0xFFFFFFFFFFFFFFFF);
    }

    var cmap: [*]mem.clubbed_entry = undefined;
    const num_entries = mem.ClubMmap(mmap, mmapSize, descSize, &cmap);
    if (num_entries == 0) {
        serial.write("Failed to club entries\r\n", .{});
        stall(0xFFFFFFFFFFFFFFFF);
    }

    sRet = paging.MapUsableMemory(cmap, num_entries, kernel);
    if (status.success != sRet) {
        serial.write("Failed to map memory\r\n", .{});
        stall(0xFFFFFFFFFFFFFFFF);
    } else {
        mem.free(@ptrCast(cmap));
    }

    sRet = paging.IdentityMapImage(loaded_img.image_base, loaded_img.image_size);
    if (status.success != sRet) {
        serial.write("Failed to setup Identity Map Image\r\n", .{});
        stall(0xFFFFFFFFFFFFFFFF);
    }

    if (status.success == mem.GetMemoryMap(&mmapSize, &mmap, &key, &descSize, &descVer)) {
        if (status.success != boot_services.exitBootServices(uefi.handle, key)) {
            serial.write("Exit boot services failed\r\n", .{});
            stall(0xFFFFFFFFFFFFFFFF);
        }
    }

    //var args: *kargs = @ptrCast(argsPage);
    const vkargs = @intFromPtr(argsPage) + paging.kMemAddr;
    serial.write("Kernel Arguments at paddr: {*} vaddr: 0x{x}\r\n", .{
        argsPage,
        vkargs,
    });
    argsPage.KPAddr = @intFromPtr(kernel);
    argsPage.KOffset = paging.kCompAddr;
    argsPage.KSize = 2 << 20;

    argsPage.KMemOffset = paging.kMemAddr;
    argsPage.KMemPages = paging.totalMemPages;

    argsPage.KMemMap = @ptrFromInt(@intFromPtr(mmap) + paging.kMemAddr);
    argsPage.KMemMapSize = mmapSize;
    argsPage.DescSize = descSize;

    argsPage.PML4 = paging.pml4.?;

    serial.write("Jumping to Kernel at 0x{x}\r\n", .{argsPage.KPAddr + argsPage.KOffset});

    asm volatile (
        \\mov %[pml4], %%rax
        \\mov %%rax, %%cr3
        \\mov %[args], %%r13
        \\jmp *%[entry]
        :
        : [pml4] "r" (paging.pml4.?),
          [entry] "r" (entry.?),
          [args] "r" (vkargs),
        : "rax"
    );
}
