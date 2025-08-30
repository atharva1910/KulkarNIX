const uefi = @import("std").os.uefi;
const MemoryMapSlice = uefi.tables.MemoryMapSlice;
const Page = uefi.Page;

pub const kargs = struct {
    // Kernel code segment
    KernelPAddr: usize,
    KCodeOffset: usize,
    KCodePages: usize,

    // Kernel Memory segment
    KDataOffset: usize,
    KDataPages: usize,

    // Memory Map
    KMemMap: MemoryMapSlice,

    // Paging
    PML4: []Page,
    NumPDPT: usize,
    NumPDT: usize,
    NumPT: usize,
};
