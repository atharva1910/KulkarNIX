#pragma once
#include "typedefs.h"
#define ENTRIES_PER_TABLE    1024

/* PD entry */
typedef union  __attribute__((packed)) _PDE {
    struct {
        uint8_t  P         :1;     // Present?
        uint8_t  RW        :1;     // Read/Write Page
        uint8_t  US        :1;     // User/Supervisor
        uint8_t  PWT       :1;     // Page Write Through
        uint8_t  PCD       :1;     // Page Cache Disable
        uint8_t  A         :1;     // Accessed
        uint8_t  IGN       :1;     // Reserved
        uint8_t  PS        :1;     // Page Size. 0 = 4K
        uint8_t  IGN_1     :1;     // Reserved
        uint8_t  AVL       :3;     // Available
        uint32_t ADDRESS   :20;    // Page Table address
    }u;
    uint32_t ui32pdEntry;
}PDE;

/* PDP & PD table */
typedef struct _PDT {
    PDE  pde[ENTRIES_PER_TABLE];
}PDT;

/* PT Entry */
typedef union  __attribute__((packed)) _PTE {
    struct {
        uint8_t  P         :1;    // Present?
        uint8_t  RW        :1;    // Read/Write Page
        uint8_t  US        :1;    // User/Supervisor
        uint8_t  PWT       :1;    // Page Write Through
        uint8_t  PCD       :1;    // Page Cache Disable
        uint8_t  A         :1;    // Accessed
        uint8_t  D         :1;    // Dirty
        uint8_t  PAT       :1;    // Page Size. 0 = 4K
        uint8_t  G         :1;    // Global
        uint8_t  AVL       :3;    // Available
        uint32_t ADDRESS   :20;   // Page Table address;
    }u;
    uint32_t ui32ptEntry;
}PTE;

/* PT table */
typedef struct _PT {
    PTE pte[ENTRIES_PER_TABLE];
}PT;
