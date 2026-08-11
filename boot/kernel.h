#pragma once

#include "efi.h"

class KernInfo {
 public:
    uintptr_t kernel_entry;
    UINTN kernel_pages;
    UINTN min_addr;
    UINTN max_addr;
    UINTN kernel_size;
};
