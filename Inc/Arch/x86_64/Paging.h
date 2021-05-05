#ifndef _KPAGING_H
#define _KPAGING_H
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
}PageMapLevel4Entry, *PPageMapLevel4Entry;

/* PML4 table */
typedef struct _PML4T{
    PageMapLevel4Entry pageMapLevel4Entry[512];
}PageMapLevel4Table, *PPageMapLevel4Table;

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
}PageDirPtrEntry, *PPageDirPtrEntry, PageDirEntry, *PPageDirEntry;

/* PDP & PD table */
typedef struct _PDPT{
    union{
        PageDirPtrEntry pageDirPtrEntry[512];
        PageDirEntry    pageDirEntry[512];
    };
}PageDirPtrTable, *PPageDirPtrTable, PageDirTable, *PPageDirTable;

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
}PageTableEntry, *PPageTableEntry;

/* PT table */
typedef struct _PTT{
    PageTableEntry pageTableEntry[512];
}PageTable, *PPageTable;

#endif
