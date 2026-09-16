#pragma once
#include <stdint.h>

/////////////////////////////////////////////////////////////////////////////////
// This file contains all common MACROS and defines needed for all the modules //
/////////////////////////////////////////////////////////////////////////////////

constexpr uint64_t KERNEL_START_PADDR = 0x100000;
constexpr uint64_t KERNEL_START_VADDR = 0xfffffa0000000000;
constexpr uint64_t HIGHER_MEMORY_VADDR = 0xFFFF800000000000;

template <typename T>
constexpr T CEILING(T x, T y) {
    return ((x + (y -1))/y);
}

template <typename T>
constexpr uint64_t PA2VA(T PA)
{
    return reinterpret_cast<uint64_t>(PA) + HIGHER_MEMORY_VADDR;
}
