#pragma once

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
