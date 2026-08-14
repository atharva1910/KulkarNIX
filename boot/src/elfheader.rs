#[repr(C)]
#[derive(Debug, Clone, Copy, Default)]
pub struct Elf64Ehdr {
    pub e_ident: [u8; 16], // Magic number and other info
    pub e_type: u16,        // Object file type
    pub e_machine: u16,     // Architecture
    pub e_version: u32,     // Object file version
    pub e_entry: u64,       // Entry point virtual address
    pub e_phoff: u64,       // Program header table file offset
    pub e_shoff: u64,       // Section header table file offset
    pub e_flags: u32,       // Processor-specific flags
    pub e_ehsize: u16,      // ELF header size in bytes
    pub e_phentsize: u16,   // Program header table entry size
    pub e_phnum: u16,       // Program header table entry count
    pub e_shentsize: u16,   // Section header table entry size
    pub e_shnum: u16,       // Section header table entry count
    pub e_shstrndx: u16,    // Section header string table index
}

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct Elf64Phdr {
    pub p_type: u32,   // Segment type
    pub p_flags: u32,  // Segment flags
    pub p_offset: u64, // Segment file offset
    pub p_vaddr: u64,  // Segment virtual address
    pub p_paddr: u64,  // Segment physical address
    pub p_filesz: u64, // Segment size in file
    pub p_memsz: u64,  // Segment size in memory
    pub p_align: u64,  // Segment alignment
}

// ELF Magic Identifier & Segment Types
pub const ELF_MAGIC: [u8; 4] = [0x7F, b'E', b'L', b'F'];
pub const PT_LOAD: u32 = 1;
