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
#[derive(Debug, Clone, Copy, Default)]
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

#[repr(C)]
#[derive(Debug, Copy, Clone, Default)]
pub struct Elf64Shdr {
    pub sh_name: u32,      // Section name (string table index)
    pub sh_type: u32,      // Section type
    pub sh_flags: u64,     // Section flags
    pub sh_addr: u64,      // Section virtual addr at execution
    pub sh_offset: u64,    // Section file offset
    pub sh_size: u64,      // Section size in bytes
    pub sh_link: u32,      // Link to another section
    pub sh_info: u32,      // Additional section information
    pub sh_addralign: u64, // Section alignment
    pub sh_entsize: u64,   // Entry size if section holds a table
}

#[repr(C)]
#[derive(Debug, Copy, Clone, Default)]
pub struct Elf64Rela {
    pub r_offset: u64,
    pub r_info: u64,
    pub r_append: u64,
}

#[repr(C)]
#[derive(Debug, Copy, Clone, Default)]
pub struct Elf64Dyn {
    pub d_tag: u64,  // Identification tag (e.g., DT_RELA, DT_RELASZ)
    pub d_val: u64,  // Represents an integer value or a virtual address
}

// ELF Magic Identifier & Segment Types
pub const ELF_MAGIC: [u8; 4] = [0x7F, b'E', b'L', b'F'];
pub const PT_LOAD: u32 = 1;
pub const PT_DYNAMIC: u32 = 2;
pub const SHT_RELA: u32 = 4;
