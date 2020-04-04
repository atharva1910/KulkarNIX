#include "HAL/Paging.h"

KPageDir::KPageDir(uint32_t addr)
{
    PDT = reinterpret_cast<uintptr_t>(addr);
    for(int i = 0; i < 1024; i++)
        PDT[i] = 0;
}

void KPageDir::CreatePageTableEntry(uint idx, KPageTable &pageTable)
{
    PDT[idx] = reinterpret_cast<uint32_t>(pageTable.GetPageTableAddress());
}

KPageTable::KPageTable(uint32_t addr)
{
    PT = reinterpret_cast<uintptr_t>(addr);
}

void KPageTable::EmptyPageTable()
{
    for(int i = 0; i < 1024; i++)
        PT[i] = 0;
}

uintptr_t KPageTable::GetPageTableAddress()
{
    return PT;
}

KPageTable::~KPageTable() {}
KPageDir::~KPageDir() {}

/*
SetupX86Paging
Description:
Function sets up X86 paging
Sets the page directory table at 0x1000 
Sets the page table at 0x2000

Arguments: 
  None
  
Output:
  None
*/
void SetupX86Paging()
{
    // Create Page Directory Table at 0x1000
    KPageDir   PageDirectory(0x1000);
    // Create Page Table at 0x2000 (8KB)
    KPageTable PageTable(0x2000);
    // Clear Page Table
    PageTable.EmptyPageTable();
    // Create entry at idx = 0 to identity map 4 MB
    PageDirectory.CreatePageTableEntry(0, PageTable);
}

