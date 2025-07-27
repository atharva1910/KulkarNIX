pub const kargs = packed struct {
    kpaddr: u64,
    kvaddr: u64,
    ksize: usize,
    pagetable: u64,
    gop_buffer: u64,
    memory_map: u64,
    memory_map_size: usize,
    memory_map_dsize: usize,
};
