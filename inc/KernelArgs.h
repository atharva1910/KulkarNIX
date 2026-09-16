#pragma once
#include <stdint.h>

struct MemMapInfo {
    uint64_t dsize;
    uint64_t size;
    uint64_t total_memory;
    uint32_t num_desc;
    uint8_t *mm;
};

struct KernelInfo {
    uint64_t kernelPages;
    uint64_t minAddr;
    uint64_t kernelSize;
};

struct KernelArgs {
    char magic[8];
    KernelInfo k_info;
    MemMapInfo mm_info;
};
