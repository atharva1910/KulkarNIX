#pragma once
#include <stdint.h>

/////////////////////////////////////////////////////////////////////////////////
// This file contains all common MACROS and defines needed for all the modules
// //
/////////////////////////////////////////////////////////////////////////////////

constexpr uint16_t PAGE_SIZE = 4096;
constexpr uint16_t PAGE_SIZE_SHIFT = 12;
constexpr uint64_t KERNEL_START_PADDR = 0x100000;
constexpr uint64_t KERNEL_START_VADDR = 0xfffffa0000000000;
constexpr uint64_t HIGHER_MEMORY_VADDR = 0xFFFF800000000000;

template <typename T>
constexpr T CEILING(T x, T y) {
    return ((x + (y -1))/y);
}

template<typename AddrType>
struct AddrBase {
    uint64_t addr;
    explicit AddrBase(uint64_t x) : addr(x) {}

    bool is_page_aligned() const { return (addr & (PAGE_SIZE - 1)) == 0; }
    uint64_t get_alignment() const { return addr & ~(PAGE_SIZE - 1); }
    void operator+(uint64_t x) { addr += x; }
    AddrBase operator+(uint64_t x) const { return AddrBase(addr + x); }
    uint64_t get_raw() const { return addr; }
    //bool operator<(uint64_t x) { return addr < x; }
    bool operator<(AddrType x) const { return addr < x.addr; }
    //bool operator>(uint64_t x) { return addr > x; }
    bool operator>(AddrType x) const { return addr > x.addr; }
    //bool operator==(uint64_t x) { return addr > x; }
    bool operator==(AddrType x) const { return addr == x.addr; }

    uint64_t print() { return addr; }
};

struct PA;
struct VA;

struct PA : AddrBase<PA>{
    using AddrBase::AddrBase;
    explicit PA(const VA& va);
};

struct VA : AddrBase<VA> {
    using AddrBase::AddrBase;
    explicit VA(const PA& pa);
};

inline PA::PA(const VA& va)
        : AddrBase<PA>(va.get_raw() - HIGHER_MEMORY_VADDR) {}

inline VA::VA(const PA& pa)
        : AddrBase<VA>(pa.get_raw() + HIGHER_MEMORY_VADDR) {}
