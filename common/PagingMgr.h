#pragma once
#include <stdint.h>
#include "paging.h"

template <typename Allocator>
class PagingMgr {
public:
 PagingMgr(Allocator &allocator) : m_allocator(allocator) {
        m_pml4t = reinterpret_cast<GenericPagingEntry *>(allocator.alloc_pages(1));
        assert(m_pml4t != 0);
    }

    GenericPagingEntry* get_or_create_entry(GenericPagingEntry *table, uint16_t index) {
        if (table[index] == nullptr) { //error??
          table[index] = reinterpret_cast<GenericPagingEntry *>(m_allocator.alloc_pages(1));
      }
        return nullptr;
    }

    bool map_page(uint64_t paddr, uint64_t vaddr) {
        assert(m_pml4t != 0);
        auto offsets = get_offsets(vaddr);
        auto pdpt = get_or_create_entry(m_pml4t, offsets.pml4_idx);
        auto pdt = get_or_create_entry(pdpt, offsets.pdpt_idx);
        auto pt = get_or_create_entry(pdt, offsets.pdt_idx);
        pt[offsets.pt_idx] = paddr;
    }

 private:
    struct entry_offsets {
        uint16_t pml4_idx;
        uint16_t pdpt_idx;
        uint16_t pdt_idx;
        uint16_t pt_idx;
    };

    entry_offsets get_offsets(uint64_t addr) {
      return entry_offsets {
          static_cast<uint16_t>((addr >> 39) & 0x1FF),
          static_cast<uint16_t>((addr >> 30) & 0x1FF),
          static_cast<uint16_t>((addr >> 21) & 0x1FF),
          static_cast<uint16_t>((addr >> 12) & 0x1FF)
      };
    }
    GenericPagingEntry *m_pml4t = nullptr;
    Allocator &m_allocator;
};
