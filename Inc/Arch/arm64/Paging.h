#pragma once
#include "typedefs.h"

#define LONG_MODE_TABLE_SIZE 512

/* PML4 entry */
typedef union  __attribute__((packed)) _PML4E{
    struct {
        uint8_t  P         :1;
        uint8_t  RW        :1;
        uint8_t  US        :1;
        uint8_t  PWT       :1;
        uint8_t  PCD       :1;
        uint8_t  A         :1;
        uint8_t  IGN       :1;
        uint8_t  MBZ       :1;
        uint8_t  MBZ_1     :1;
        uint8_t  AVL       :3;
        uint64_t ADDRESS   :40;
        uint16_t AVAILABLE :11;
        uint8_t  NX        :1;
    }u;
    uint64_t ui64pml4Entry;
}PML4E;

/* PML4 table */
typedef struct _PML4T{
    PML4E pml4e[512];
}PML4T;

/* PDP entry & PD entry */
typedef union  __attribute__((packed)) _PDPE{
    struct {
        uint8_t  P         :1;
        uint8_t  RW        :1;
        uint8_t  US        :1;
        uint8_t  PWT       :1;
        uint8_t  PCD       :1;
        uint8_t  A         :1;
        uint8_t  IGN       :1;
        uint8_t  RES       :1;
        uint8_t  IGN_1     :1;
        uint8_t  AVL       :3;
        uint64_t ADDRESS   :40;
        uint16_t AVAILABLE :11;
        uint8_t  NX        :1;
    }u;
    union {
        uint64_t ui64pdpEntry;
        uint64_t ui64pdEntry;
    };
}PDPE, PDE;

/* PDP & PD table */
typedef struct _PDPT{
    union{
        PDPE pdpe[512];
        PDE  pde[512];
    };
}PDPT, PDT;

/* PT Entry */
typedef union  __attribute__((packed)) _PTE{
    struct {
        uint8_t  P         :1;
        uint8_t  RW        :1;
        uint8_t  US        :1;
        uint8_t  PWT       :1;
        uint8_t  PCD       :1;
        uint8_t  A         :1;
        uint8_t  D         :1;
        uint8_t  PAT       :1;
        uint8_t  G         :1;
        uint8_t  AVL       :3;
        uint64_t ADDRESS   :40;
        uint16_t  AVAILABLE :7;
        uint8_t  PKE       :4;
        uint8_t  NX        :1;
    }u;
    uint64_t ui64ptEntry;
}PTE;

/* PT table */
typedef struct _PTT{
    PTE pte[512];
}PT;
