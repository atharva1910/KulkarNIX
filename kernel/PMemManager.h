#pragma once
#include "KernelArgs.h"

namespace PMM {
    bool init(const KernelArgs* kernelArgs);
    void* alloc();
    void free(void *p);
}
