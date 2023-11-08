#include "HAL/HAL.h"
//#include "x86lib.h"

namespace HAL {
void EnableInterrupts()
{
  asm volatile("sti");
}

void DisableInterrupts()
{
  asm volatile("cli");
}

BOOL CheckIfApicExists()
{
    uint32_t edx = 0, a = 1;
    asm volatile("cpuid": "=d" (edx): "a" (a));
    if (edx & 0x200)
        return true;
    return false;

}

BOOL CheckIfCpuidExists()
{
    BOOL bRet = true;
    return bRet;
}

void EnablePaging(uint32_t PDT)
{
    // Set the pointer to PDT in CR3
    __asm__ volatile("mov %0, %%cr3"::"r"((uint64_t)PDT));
    // Enable flag in CR0
    /*    asm volatile("mov %%cr0, %%eax\n\t    \
                  or  $0x80000001, %%eax\n\t\
                  mov %%eax, %%cr0" :::"%eax");*/
}

} // namespace x86
