#ifndef _KPAGING_H
#define _KPAGING_H
#include "typedefs.h"

void SetupX64Paging();
void SetupX86Paging();

class KPageTable{
 public:
    KPageTable(uint32_t addr);
    ~KPageTable();

    void      EmptyPageTable();
    uintptr_t GetPageTableAddress();
 private:
    uint32_t *PT = nullptr;
};

class KPageDir{
 public:
    KPageDir(uint32_t addr);
    ~KPageDir();

    void CreatePageTableEntry(uint idx, KPageTable &pageTable);
 private:
    uint32_t *PDT = nullptr;
};

#endif
