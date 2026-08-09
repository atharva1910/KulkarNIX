#pragma once

// PML4 Entry (Points to PDP Table)
constexpr uint32_t PAGE_TABLE_NUM_ENTRIES = 512;
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
        uint64_t IGN2      : 1;  // Bit 8: Ignored
        uint64_t AVL       : 3;  // Bits 9-11: Available for OS
        uint64_t pdpt      : 40; // Bits 12-51: Physical Address of PDPT (>> 12)
        uint64_t Available : 11; // Bits 52-62: Available for OS
        uint64_t NX        : 1;  // Bit 63: No Execute
    } __attribute__((packed));

    uint64_t raw;
};

// PDP Entry (Points to PD Table OR 1 GiB Page)
union alignas(8) PDPE {
    struct {
        uint64_t P         : 1;  // Bit 0: Present
        uint64_t RW        : 1;  // Bit 1: Read/Write
        uint64_t US        : 1;  // Bit 2: User/Supervisor
        uint64_t PWT       : 1;  // Bit 3: Page-level write-through
        uint64_t PCD       : 1;  // Bit 4: Page-level cache disable
        uint64_t A         : 1;  // Bit 5: Accessed
        uint64_t PS        : 1;  // Bit 6: Page Size (0 = points to PDT, 1 = 1 GiB page)
        uint64_t MBZ       : 1;  // Bit 7: Must be zero (if PS = 0)
        uint64_t IGN       : 1;  // Bit 8: Ignored
        uint64_t AVL       : 3;  // Bits 9-11: Available for OS
        uint64_t pdt       : 40; // Bits 12-51: Physical Address of PDT (>> 12)
        uint64_t Available : 11; // Bits 52-62: Available for OS
        uint64_t NX        : 1;  // Bit 63: No Execute
    } __attribute__((packed));

    uint64_t raw;
};

union alignas(8) PDE {
    // Mode A: PS = 0 (Points to a 4 KB Page Table)
    struct {
        uint64_t P         : 1;  // Bit 0: Present
        uint64_t RW        : 1;  // Bit 1: Read/Write
        uint64_t US        : 1;  // Bit 2: User/Supervisor
        uint64_t PWT       : 1;  // Bit 3: Page-level write-through
        uint64_t PCD       : 1;  // Bit 4: Page-level cache disable
        uint64_t A         : 1;  // Bit 5: Accessed
        uint64_t IGN1      : 1;  // Bit 6: Ignored
        uint64_t PS        : 1;  // Bit 7: Must be 0 for 4 KB Page Table
        uint64_t IGN2      : 1;  // Bit 8: Ignored
        uint64_t AVL       : 3;  // Bits 9-11: Available for OS
        uint64_t pt        : 40; // Bits 12-51: Physical address of PT (>> 12)
        uint64_t Available : 11; // Bits 52-62: Available for OS
        uint64_t NX        : 1;  // Bit 63: No Execute
    } __attribute__((packed)) table;

    // Mode B: PS = 1 (Maps a direct 2 MB Large Page)
    struct {
        uint64_t P         : 1;  // Bit 0: Present
        uint64_t RW        : 1;  // Bit 1: Read/Write
        uint64_t US        : 1;  // Bit 2: User/Supervisor
        uint64_t PWT       : 1;  // Bit 3: Page-level write-through
        uint64_t PCD       : 1;  // Bit 4: Page-level cache disable
        uint64_t A         : 1;  // Bit 5: Accessed
        uint64_t D         : 1;  // Bit 6: Dirty
        uint64_t PS        : 1;  // Bit 7: Must be 1 for 2 MB Page
        uint64_t G         : 1;  // Bit 8: Global Page
        uint64_t AVL_Low   : 3;  // Bits 9-11: Available for OS
        uint64_t PAT       : 1;  // Bit 12: Page Attribute Table
        uint64_t Reserved  : 8;  // Bits 13-20: Reserved (Must be 0)
        uint64_t page_2mb  : 31; // Bits 21-51: Physical Address of 2MB Page (>> 21)
        uint64_t AVL_High  : 11; // Bits 52-62: Available for OS
        uint64_t NX        : 1;  // Bit 63: No Execute
    } __attribute__((packed)) page2mb;

    uint64_t raw;
};
