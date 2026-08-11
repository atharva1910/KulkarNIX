#pragma once

class MemoryMap {
public:
    MemoryMap() {}
    uint64_t key = 0;
    uint64_t dsize = 0;
    uint64_t size = 0;
    uint32_t dver = 0;
    uint32_t num_desc = 0;
    uint8_t *mm = nullptr;
    uint64_t min_paddr = -1, max_paddr = 0;
    uint64_t min_vaddr = -1, max_vaddr = 0;
    UINT64 total_mem = 0;

};
