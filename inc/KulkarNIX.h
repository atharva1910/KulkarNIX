#pragma once
#include <stdint.h>
#include <stddef.h>


/////////////////////////////////////////////////////////////////////////////////
// This file contains all common MACROS and defines needed for all the modules
// //
/////////////////////////////////////////////////////////////////////////////////

constexpr uint16_t PAGE_SIZE = 4096;
constexpr uint16_t PAGE_SIZE_SHIFT = 12;
constexpr uintptr_t KERNEL_START_PADDR = 0x100000;
constexpr uintptr_t KERNEL_START_VADDR = 0xfffffa0000000000;
constexpr uintptr_t HIGHER_MEMORY_VADDR = 0xFFFF800000000000;

template <typename T>
constexpr T CEILING(T x, T y) {
    return ((x + (y -1))/y);
}

template <typename Parent, typename Member>
size_t OFFSET_OF(Member Parent::* member_ptr) {
    return reinterpret_cast<uintptr_t>(&(((Parent*)nullptr)->*member_ptr));
}

template <typename Parent, typename Member>
Parent* CONTAINER_OF(Member* member, Member Parent::* member_ptr) {
    if (member == nullptr)
        return nullptr;

    uintptr_t offset = reinterpret_cast<uintptr_t>(&(((Parent*)nullptr)->*member_ptr));
    uintptr_t mem_addr = reinterpret_cast<uintptr_t>(member);
    return reinterpret_cast<Parent *>(mem_addr - offset);
}


template<typename AddrType>
struct AddrBase {
    uintptr_t addr;
    explicit AddrBase(uintptr_t x) : addr(x) {}

    bool is_page_aligned() const { return (addr & (PAGE_SIZE - 1)) == 0; }
    uint64_t get_alignment() const { return addr & ~(PAGE_SIZE - 1); }
    void operator+(uintptr_t x) { addr += x; }
    AddrBase operator+(uintptr_t x) const { return AddrBase(addr + x); }
    uintptr_t get_raw() const { return addr; }
    //bool operator<(uint64_t x) { return addr < x; }
    bool operator<(AddrType x) const { return addr < x.addr; }
    //bool operator>(uint64_t x) { return addr > x; }
    bool operator>(AddrType x) const { return addr > x.addr; }
    //bool operator==(uint64_t x) { return addr > x; }
    bool operator==(AddrType x) const { return addr == x.addr; }
    uintptr_t operator>>(int x) const { return addr >> x; }
    uintptr_t operator<<(int x) const { return addr << x; }

    uintptr_t print() { return addr; }
};

struct PA;
struct VA;

struct PA : AddrBase<PA>{
    using AddrBase::AddrBase;
    explicit PA(const VA& va);
    //VA to_virtual();
};

struct VA : AddrBase<VA> {
    using AddrBase::AddrBase;
    explicit VA(const PA& pa);
    //PA to_physical();
};

inline PA::PA(const VA& va)
        : AddrBase<PA>(va.get_raw() - HIGHER_MEMORY_VADDR) {}

inline VA::VA(const PA& pa)
        : AddrBase<VA>(pa.get_raw() + HIGHER_MEMORY_VADDR) {}
