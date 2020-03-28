#include "Paging.h"

void SetupX64Paging()
{
    // The paging directory will be at 0x1000
    uint32_t *pageDirBase = (uint32_t *)0x1000;
    // clear area for 4 tables
    for(uint32_t i = 0; i < 512 * 4; i++)
        pageDirBase[i] = 0;

    // move address of pm4l into cr3
    asm volatile("")
}
