#pragma once
#include <typedefs.h>

typedef struct _MMapEntry {
    uint32_t addr_low;
    uint32_t addr_high;
    uint32_t len_low;
    uint32_t len_high;            
    uint32_t type;
    uint32_t acpi;    
}MMapEntry, *PMMapEntry;

uint32_t
GetMemMapEntries();

PMMapEntry
GetMemMap();

