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
const openMode = uefi.protocol.File.OpenMode;
const MemoryMapSlice = @import("std").os.uefi.tables.MemoryMapSlice;
pub var boot_services: *uefi.tables.BootServices = undefined;
var loaded_img: ?*uefi.protocol.LoadedImage = undefined;
var gop: ?*uefi.protocol.GraphicsOutput = undefined;

fn stall(ms: u64) void {
    _ = uefi.system_table.boot_services.?.stall(ms) catch {};
}

fn ReadKernel(pages: []u8) !u64 {
    loaded_img = try boot_services.handleProtocol(uefi.protocol.LoadedImage, uefi.handle);

    var sfs = try boot_services.handleProtocol(uefi.protocol.SimpleFileSystem, loaded_img.?.device_handle.?);

    var fileProt = try sfs.?.openVolume();

    var kHandle = try fileProt.open(L("\\Kernel.elf"), openMode.read, .{
        .read_only = true,
    });
    defer _ = fileProt.close() catch {};

    var elf_hdr: elf.Ehdr = undefined;
    if (try kHandle.read(@ptrCast(&elf_hdr)) != @sizeOf(elf.Ehdr)) {
        serial.write("Failed to read Kernel.elf\r\n", .{});
        return uefi.Error.Aborted;
    }

    try kHandle.setPosition(elf_hdr.e_phoff);

    var p_phdr: []elf.Elf64_Phdr = @ptrCast(try mem.alloc(elf_hdr.e_phnum * elf_hdr.e_phentsize));
    defer mem.free(@ptrCast(p_phdr));

    if (try kHandle.read(@ptrCast(p_phdr[0..])) != (elf_hdr.e_phnum * elf_hdr.e_phentsize)) {
        //serial.write("Failed to read Phdrs\r\n", .{});
        return uefi.Error.Aborted;
    }

    for (0..elf_hdr.e_phnum) |i| {
        if (p_phdr[i].p_type != elf.PT_LOAD)
            continue;

        const buf_idx = p_phdr[i].p_paddr - kCompAddr;
        try kHandle.setPosition(p_phdr[i].p_offset);

        if (try kHandle.read(pages[buf_idx .. buf_idx + p_phdr[i].p_filesz]) < p_phdr[i].p_filesz) {
            break;
        } else {
            serial.write("phdr[{}] addr {*} size 0x{x}\r\n", .{ i, &pages[buf_idx], p_phdr[i].p_filesz });
        }
    }

    return elf_hdr.e_entry;
}

fn InitGOP() !void {
    gop = try boot_services.locateProtocol(uefi.protocol.GraphicsOutput, null);

    var smode: u32 = undefined;
    for (0..gop.?.mode.max_mode) |i| {
        const mode: u32 = @intCast(i);
        const info = try gop.?.queryMode(mode);
        if (info.horizontal_resolution == 800 and info.vertical_resolution == 600) {
            smode = mode;
            //serial.write("Setting GOP mode {} {}x{}\r\n", .{ i, info.horizontal_resolution, info.vertical_resolution });
            break;
        }
    }

    try gop.?.setMode(smode);
}

pub fn main() uefi.Error!void {
    boot_services = uefi.system_table.boot_services.?;

    screen.init();
    screen.clrscr();
    try serial.init();

    var argsPage: *kargs = @ptrCast(try mem.alloc_pages(1));
    const kernel = try mem.alloc_pages(512);
    const entry = try ReadKernel(@ptrCast(kernel[0..]));
    serial.write("Kernel loaded at {*}\n", .{kernel.ptr});

    try InitGOP();
    try boot_services.setWatchdogTimer(0, 0, null);

    var memSlice = mem.GetMemoryMap() catch {
        serial.write("Failed to get mmap", .{});
        stall(0xFFFFFFFFFFFFFFF);
        unreachable;
    };

    try paging.MapUsableMemory(memSlice, @intFromPtr(kernel.ptr));
    try paging.IdentityMapImage(loaded_img.?.image_base[0..loaded_img.?.image_size]);

    memSlice = try mem.GetMemoryMap();
    try boot_services.exitBootServices(uefi.handle, memSlice.info.key);

    const vkargs = @intFromPtr(argsPage) + paging.kMemAddr;
    serial.write("Kernel Arguments at paddr: {*} vaddr: 0x{x}\r\n", .{ argsPage, vkargs });

    serial.write("PML4: {*}\r\n", .{paging.pml4.?});

    argsPage.KernelPAddr = @intFromPtr(kernel.ptr);
    argsPage.KCodeOffset = paging.kCompAddr;
    argsPage.KCodePages = 1;

    argsPage.KDataOffset = paging.kMemAddr;
    argsPage.KDataPages = paging.totalMemPages;

    argsPage.KMemMap = memSlice;
    // Update the ptr from Pmem to Vmem
    argsPage.KMemMap.ptr = @ptrFromInt(@intFromPtr(memSlice.ptr) + paging.kMemAddr);

    // Page Tables
    argsPage.PageTables = paging.PageTables;
    argsPage.PageTables.ptr = @ptrFromInt(@intFromPtr(paging.PageTables.ptr) + paging.kMemAddr);
    serial.write("Changes the Page Tables ptr from {*} to {*} {}\n", .{ paging.PageTables.ptr, argsPage.PageTables.ptr, argsPage.PageTables.len });

    if (paging.isPagePresent(vkargs)) {
        serial.write("Jumping to Kernel at 0x{x}\r\n", .{argsPage.KernelPAddr + argsPage.KCodeOffset});
    } else {
        serial.write("Args page not preset\n", .{});
    }

    asm volatile (
        \\mov %[pml4], %%rax
        \\mov %%rax, %%cr3
        \\mov %[args], %%r13
        \\jmp *%[entry]
        :
        : [pml4] "r" (paging.pml4.?),
          [entry] "r" (entry),
          [args] "r" (vkargs),
        : .{
          .rax = true,
          .r13 = true,
        });
}
