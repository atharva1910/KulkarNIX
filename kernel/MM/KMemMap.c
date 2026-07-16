#include "KMemMap.h"

#define KMEM_MAP_ADDR 0x7E10
#define KMEM_MAP_ENTRIES 0x7E00

uint32_t
GetMemMapEntries()
{
    return (uint32_t)(*(uint32_t *)KMEM_MAP_ENTRIES);    
}

PMMapEntry
GetMemMap()
{
    return (PMMapEntry)KMEM_MAP_ADDR;
}

