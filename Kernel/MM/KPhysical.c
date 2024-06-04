#include "KPhysical.h"

void
InitPageFrameAllocator()
{
    uint32_t numEntries = GetMemMapEntries();
    PMMapEntry pEntry   = GetMemMap();

    for (int i = 0; i < numEntries; i++) {
        KPrint(KVERB, "[%d] Address Low: 0x%x  Address High: 0x%x Length: 0x%x Type: %d",
               i, pEntry->addr_low, pEntry->addr_high, pEntry->len_low, pEntry->type);
        pEntry++;
    }
}
