#pragma once
#include "KulkarNIX.h"

namespace Heap {
    bool init();
    void* alloc(size_t size);
    void free(void *p);
}
