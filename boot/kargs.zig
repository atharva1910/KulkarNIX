const MemoryMapSlice = @import("std").os.uefi.tables.MemoryMapSlice;

pub const kargs = struct {
    // Kernel code segment
    KPAddr: usize,
    KOffset: usize,
    KSize: usize,

    // Kernel Memory segment
    KMemOffset: usize,
    KMemPages: usize,

    // Memory Map
    KMemMap: MemoryMapSlice,

    // Paging
    PML4: [*]u64,
    NumPDPT: u32,
    PDPT: [*]u64,
    NumPDT: u32,
    PDT: [*]u64,
    NumPT: u32,
    PT: [*]u64,
};
