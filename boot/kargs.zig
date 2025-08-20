pub const kargs = packed struct {
    kpaddr: usize, // kernel paddr
    kvaddr: usize, // kernel vaddr
    ksize: usize, // kernel size
    kmemory: usize, // mapped memory "kMemAddr"
    kvoffset: usize, // kMemAddr
    pagetable: usize, // Not mapped yet
    gop_buffer: usize, //Not mapped yet
    //memory_map: usize,
    //memory_map_size: usize,
    //memory_map_dsize: usize,
};
