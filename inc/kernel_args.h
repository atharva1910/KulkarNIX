#pragma once
#include <stdint.h>

struct KernelArgs {
    char magic[10];
    struct {
        uint64_t kernelPages;
        uint64_t minAddr;
        uint64_t kernelSize;
    } kernel_info;

    struct {
        uint64_t dsize;
        uint64_t size;
        uint32_t num_desc;
        uint8_t *mm;
    } mem_map_info;
};
