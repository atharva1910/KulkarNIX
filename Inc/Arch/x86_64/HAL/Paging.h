#ifndef _KPAGING_H
#define _KPAGING_H
#include "typedefs.h"

void SetupX64Paging();
void SetupX86Paging();

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
