#pragma once

#include "typedefs.h"

typedef struct _MemMapEntry {
    uint64_t base;
    uint64_t length;    
    uint32_t type;
    uint32_t exAttr;
} MemMapEntry, *PMemMapEntry;

PMemMapEntry
GetMemMapEntry()
{
    return (PMemMapEntry)0x7E00;
}

