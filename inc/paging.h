#pragma once

constexpr uint32_t PAGE_TABLE_NUM_ENTRIES = 512;
constexpr UINT64 PAGE_SIZE = 4096;

union alignas(8) PML5E {
    struct {
        uint64_t P         : 1;  // Bit 0: Present
        uint64_t RW        : 1;  // Bit 1: Read/Write
        uint64_t US        : 1;  // Bit 2: User/Supervisor
        uint64_t PWT       : 1;  // Bit 3: Page-level write-through
        uint64_t PCD       : 1;  // Bit 4: Page-level cache disable
        uint64_t A         : 1;  // Bit 5: Accessed
        uint64_t IGN1      : 1;  // Bit 6: Ignored
        uint64_t MBZ       : 1;  // Bit 7: Must be zero
        uint64_t IGN2      : 2;  // Bit 8-10: Ignored
        uint64_t R         : 1;  // Bit 11: Ignored/HLAT
        uint64_t PDPT      : 40; // Bits 12-51: Physical Address of PDPT (>> 12)
        uint64_t IGN3      : 11; // Bits 52-62: Ignored
        uint64_t XD        : 1;  // Bit 63: Execute Disable
    } __attribute__((packed));
    uint64_t raw;
};

// PML4 Entry (Points to PDP Table)
union alignas(8) PML4E {
    struct {
        uint64_t P         : 1;  // Bit 0: Present
        uint64_t RW        : 1;  // Bit 1: Read/Write
        uint64_t US        : 1;  // Bit 2: User/Supervisor
        uint64_t PWT       : 1;  // Bit 3: Page-level write-through
        uint64_t PCD       : 1;  // Bit 4: Page-level cache disable
        uint64_t A         : 1;  // Bit 5: Accessed
        uint64_t IGN1      : 1;  // Bit 6: Ignored
        uint64_t MBZ       : 1;  // Bit 7: Must be zero
        uint64_t IGN2      : 2;  // Bit 8-10: Ignored
        uint64_t R         : 1;  // Bit 11: Ignored/HLAT
        uint64_t PDPT      : 40; // Bits 12-51: Physical Address of PDPT (>> 12)
        uint64_t IGN3      : 11; // Bits 52-62: Ignored
        uint64_t XD        : 1;  // Bit 63: Execute Disable
    } __attribute__((packed));
    uint64_t raw;
};

// PDP Entry (Points to PD Table OR 1 GiB Page)
union alignas(8) PDPTE {
    // 1 GB
    struct {
        uint64_t P         : 1;  // Bit 0: Present
        uint64_t RW        : 1;  // Bit 1: Read/Write
        uint64_t US        : 1;  // Bit 2: User/Supervisor
        uint64_t PWT       : 1;  // Bit 3: Page-level write-through
        uint64_t PCD       : 1;  // Bit 4: Page-level cache disable
        uint64_t A         : 1;  // Bit 5: Accessed
        uint64_t D         : 1;  // Bit 6: Dirty
        uint64_t PS        : 1;  // Bit 7: Page Size (0 = points to PDT, 1 = 1 GiB page)
        uint64_t G         : 1;  // Bit 8: Global
        uint64_t IGN1      : 2;  // Bit 9-10: Ignored
        uint64_t R         : 1;  // Bit 11: Ignored/HLAT
        uint64_t PAT       : 1;  // Bit 12: Memory Type
        uint64_t IGN2      : 17; // Bit 13-29: Ignored
        uint64_t PDT       : 22; // Bits 30-51: Physical Address of PDT
        uint64_t IGN3      : 7;  // Bits 52-58: Available for OS
        uint64_t PK        : 4;  // Bits 52-58: Protection Key/Ignored
        uint64_t XD        : 1;  // Bit 63: No Execute
    } __attribute__((packed)) pdpe_1gb;

    struct {
        uint64_t P         : 1;  // Bit 0: Present
        uint64_t RW        : 1;  // Bit 1: Read/Write
        uint64_t US        : 1;  // Bit 2: User/Supervisor
        uint64_t PWT       : 1;  // Bit 3: Page-level write-through
        uint64_t PCD       : 1;  // Bit 4: Page-level cache disable
        uint64_t A         : 1;  // Bit 5: Accessed
        uint64_t IGN1      : 1;  // Bit 6: Ignored
        uint64_t PS        : 1;  // Bit 7: Page Size (0 = points to PDT, 1 = 1 GiB page)
        uint64_t IGN2      : 3;  // Bit 8-10: Must be zero (if PS = 0)
        uint64_t R         : 1;  // Bit 11: Ignored/HLAT
        uint64_t PDT       : 40; // Bits 12-51: Physical Address of PDT (>> 12)
        uint64_t IGN3      : 11; // Bits 52-62: Ignored
        uint64_t XD        : 1;  // Bit 63: No Execute
    } __attribute__((packed)) pdpe;

    uint64_t raw;
};

union alignas(8) PDE {
    // 2MB
    struct {
        uint64_t P         : 1;  // Bit 0: Present
        uint64_t RW        : 1;  // Bit 1: Read/Write
        uint64_t US        : 1;  // Bit 2: User/Supervisor
        uint64_t PWT       : 1;  // Bit 3: Page-level write-through
        uint64_t PCD       : 1;  // Bit 4: Page-level cache disable
        uint64_t A         : 1;  // Bit 5: Accessed
        uint64_t D         : 1;  // Bit 6: Dirty
        uint64_t PS        : 1;  // Bit 7: Must be 0 for 4 KB Page Table, 1 for 2MB
        uint64_t G         : 1;  // Bit 8: Global
        uint64_t IGN1      : 2;  // Bit 9-10: Ignored
        uint64_t R         : 1;  // Bit 11: Ignored/HLAT
        uint64_t PAT       : 1;  // Bit 12: Memory Type
        uint64_t MBZ       : 8;  // Bits 13-20: Must be Zero
        uint64_t PT        : 31; // Bits 21-51: Physical address of 2MB PT
        uint64_t IGN2      : 7;  // Bits 52-58: Ignored
        uint64_t PK        : 4;  // Bits 59-62: Protection Key/Ignored
        uint64_t XD        : 1;  // Bit 63: No Execute
    } __attribute__((packed)) pde_2mb;

    struct {
        uint64_t P         : 1;  // Bit 0: Present
        uint64_t RW        : 1;  // Bit 1: Read/Write
        uint64_t US        : 1;  // Bit 2: User/Supervisor
        uint64_t PWT       : 1;  // Bit 3: Page-level write-through
        uint64_t PCD       : 1;  // Bit 4: Page-level cache disable
        uint64_t A         : 1;  // Bit 5: Accessed
        uint64_t IGN       : 1;  // Bit 6: Ignored
        uint64_t PS        : 1;  // Bit 7: Must be 1 for 2 MB Page, 0 for 4KB
        uint64_t IGN1      : 2;  // Bit 8-10: Ignored
        uint64_t R         : 1;  // Bit 11: Ignored/HLAT
        uint64_t AVL_Low   : 3;  // Bits 9-11: Available for OS
        uint64_t PT        : 40; // Bit 12-51: PDT
        uint64_t IGN2      : 11; // Bits 52-62: Ignored
        uint64_t XD        : 1;  // Bit 63: No Execute
    } __attribute__((packed)) pde;

    uint64_t raw;
};

union alignas(8) PTE {
    struct {
        uint64_t P         : 1;  // Bit 0: Present
        uint64_t RW        : 1;  // Bit 1: Read/Write
        uint64_t US        : 1;  // Bit 2: User/Supervisor
        uint64_t PWT       : 1;  // Bit 3: Page-level write-through
        uint64_t PCD       : 1;  // Bit 4: Page-level cache disable
        uint64_t A         : 1;  // Bit 5: Accessed
        uint64_t D         : 1;  // Bit 6: Dirty
        uint64_t PAT       : 1;  // Bit 7: Memory Type
        uint64_t G         : 1;  // Bit 8: Global
        uint64_t IGN1      : 2;  // Bit 9-10: Ignored
        uint64_t R         : 1;  // Bit 11: Ignored/HLAT
        uint64_t PAGE      : 40; // Bit 12-51: PDT
        uint64_t IGN2      : 7;  // Bits 52-58: Available for OS
        uint64_t PK        : 4;  // Bits 52-58: Protection Key/Ignored
        uint64_t XD        : 1;  // Bit 63: No Execute
    } __attribute__((packed));

    uint64_t raw;
};
