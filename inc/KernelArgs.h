#pragma once
#include <KulkarNIX.h>
#include <stdint.h>

struct MemMapInfo {
    uint64_t dsize;
    uint64_t size;
    uint64_t total_memory;
    uint32_t num_desc;
    PA min_paddr;
    PA max_paddr;
    PA mm;
};

struct KernelInfo {
    uint64_t kernelPages;
    PA kernelPAddr;
};

struct KernelArgs {
    char magic[8];
    KernelInfo k_info;
    MemMapInfo mm_info;
};
