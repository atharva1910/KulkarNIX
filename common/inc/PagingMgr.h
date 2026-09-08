#pragma once

template <typename T>
class PagingMgr {
public:
    PagingMgr(T allocator): page_allocator(allocator) {}

private:
    uint64_t m_pml4t;
    T page_allocator;
};
