#pragma once
#include <KulkarNIX.h>
#include <stdint.h>

struct MemMapInfo {
    uint64_t min_paddr;
    uint64_t max_paddr;
    uint64_t dsize;
    uint64_t size;
    uint64_t total_memory;
    uint32_t num_desc;
    PA mm;
};

struct KernelInfo {
    uint64_t kernelPages;
    PA kernelPAddr;
    VA kernelVAddr;
};

struct KernelArgs {
    char magic[8];
    KernelInfo k_info;
    MemMapInfo mm_info;
};
