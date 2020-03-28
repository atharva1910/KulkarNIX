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
} // namespace x86
