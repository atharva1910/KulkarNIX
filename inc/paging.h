#pragma once

union PML4E {
    struct u {
        uint8_t NX:1;
        uint16_t Available:11;
        uint64_t pdpt:40;
        uint8_t AVL:3;
        uint8_t MBZ:2;
        uint8_t IGN:1;
        uint8_t A:1;
        uint8_t PCD:1;
        uint8_t PWT:1;
        uint8_t US:1;
        uint8_t RW:1;
        uint8_t P:1;
    };
    uint64_t pml4e;
};

union PDPE {
    struct u {
        uint8_t NX:1;
        uint16_t Available:11;
        uint64_t pdt:40;
        uint8_t AVL:3;
        uint8_t IGN2:1;
        uint8_t RES : 1;
        uint8_t IGN1:1;
        uint8_t A:1;
        uint8_t PCD:1;
        uint8_t PWT:1;
        uint8_t US:1;
        uint8_t RW:1;
        uint8_t P:1;
    };
    uint64_t pml4e;
};

union PDE {
    struct u {
        uint8_t NX:1;
        uint16_t Available:11;
        uint64_t pt:40;
        uint8_t AVL:3;
        uint8_t IGN2:1;
        uint8_t RES : 1;
        uint8_t IGN1:1;
        uint8_t A:1;
        uint8_t PCD:1;
        uint8_t PWT:1;
        uint8_t US:1;
        uint8_t RW:1;
        uint8_t P:1;
    };
    uint64_t pml4e;
};

union PTE {
    struct u {
        uint8_t NX:1;
        uint16_t Available:11;
        uint64_t phy_page_addr:40;
        uint8_t AVL:3;
        uint8_t G:1;
        uint8_t PAT : 1;
        uint8_t D:1;
        uint8_t A:1;
        uint8_t PCD:1;
        uint8_t PWT:1;
        uint8_t US:1;
        uint8_t RW:1;
        uint8_t P:1;
    };
    uint64_t pml4e;
};
