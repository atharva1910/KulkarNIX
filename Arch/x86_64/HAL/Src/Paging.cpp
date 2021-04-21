#include "HAL/Paging.h"
#include "HAL/x86.h"

// Page Dirs
KPageDir::KPageDir(uint32_t addr)
{
    PDT = reinterpret_cast<uintptr_t>(addr);
    for(int i = 0; i < 1024; i++)
        PDT[i] = 0;
}

void KPageDir::CreatePageDirEntry(uint idx, KPageTable &pageTable)
{
    PDT[idx] = reinterpret_cast<uint64_t>(pageTable.GetPageTableAddress()) | 0x3;   // r/w, P
}

KPageDir::~KPageDir() {}

// Page Table

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

void KPageTable::CreatePageTableEntry(uint idx, uint32_t address)
{
    if (address & 0xFFF){
        // should really throw an error here
        return;
    }

    PT[idx] = address | 3;
}

KPageTable::~KPageTable() {}


// Interfaces
void IdentityMap2MB(KPageTable &pageTable)
{
    // Each page holds 4KB, at each index store increments of 4KB
    uint32_t address = 0;
    for(uint i = 0; i < 512; i++, address += 0x1000){
        pageTable.CreatePageTableEntry(i, address);
    }
}

void EnablePaging()
{
    HAL::EnablePaging(0x1000);
}

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
    // Create entry at idx = 0 to identity map 2 MB
    PageDirectory.CreatePageDirEntry(0, PageTable);
    IdentityMap2MB(PageTable);
    EnablePaging();
}

