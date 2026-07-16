#ifndef _KPAGING_H
#define _KPAGING_H
#include "typedefs.h"

void SetupX64Paging();
void SetupX86Paging();

typedef struct  __attribute__((packed)) _PML4E{
    union{
        struct {
            uint8_t p:1;
            uint8_t rw:1;
            uint8_t us:1;
            uint8_t pwt:1;
            uint8_t pcd:1;
            uint8_t a:1;
            uint8_t ign:1;
            uint8_t mbz:2;
            uint8_t avl:3;
            uint64_t address:40;
            uint32_t available:11;
            uint8_t nx:1;
        }u;
        uint64_t ui64_pml4e;
    };
}PML4E, *PPML4E;

typedef struct  __attribute__((packed)) _PDPE{
    union{
        struct {
            uint8_t p:1;
            uint8_t rw:1;
            uint8_t us:1;
            uint8_t pwt:1;
            uint8_t pcd:1;
            uint8_t a:1;
            uint8_t ign:1;
            uint8_t res:1;
            uint8_t ign_1:1;
            uint8_t avl:3;
            uint64_t address:40;
            uint32_t available:11;
            uint8_t nx:1;
        }u;
        union {
            uint64_t ui64_pdpe;
            uint64_t ui64_pde;
        };
    };
}PDPE, *PPDPE, PDE, *PPDE;

typedef struct  __attribute__((packed)) _PTE{
    union{
        struct {
            uint8_t p:1;
            uint8_t rw:1;
            uint8_t us:1;
            uint8_t pwt:1;
            uint8_t pcd:1;
            uint8_t a:1;
            uint8_t d:1;
            uint8_t pat:1;
            uint8_t g:1;
            uint8_t avl:3;
            uint64_t address:40;
            uint8_t available:7;
            uint8_t pke:4;
            uint8_t nx:1;
        }u;
        uint64_t ui64_pml4e;
    };
}PTE, *PPTE;

class KPageTable{
 public:
    KPageTable(uint64_t addr);
    ~KPageTable();

    void      EmptyPageTable();
    uintptr_t GetPageTableAddress();
    void      CreatePageTableEntry(uint32_t idx, uint64_t address);
 private:
    uint64_t *PT = nullptr;
};

class KPageDir{
 public:
    KPageDir(uint64_t addr);
    ~KPageDir();

    void CreatePageDirEntry(uint32_t idx, KPageTable &pageTable);
 private:
    uint64_t *PDT = nullptr;
};

#endif
