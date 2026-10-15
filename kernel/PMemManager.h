#pragma once
#include "KernelArgs.h"

namespace PMM {
    bool init(const KernelArgs* kernelArgs);
    void* alloc_pages(size_t n);
    void free(void *p);
}
