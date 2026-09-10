#pragma once
#include "paging.h"

using PAGE_ALLOCATOR = void *(*)(uint64_t);

template <typename Allocator> class PagingManager {
public:
  PagingManager(Allocator allocator) : m_allocator(allocator) {
    m_pml4t = reinterpret_cast<PageTable *>(m_allocator(1));
  }

  PageTable *get_or_create_table(PageTable *table, uint16_t idx) const {
    auto &pte = table->get(idx);
    if (pte.is_clear()) {
      // TODO check if allocation is successful. Ignore for now
      auto page = reinterpret_cast<uint64_t>(m_allocator(1));
      pte.set_raw(page);
      pte.set_present();
      pte.set_rw();
    }

    return reinterpret_cast<PageTable *>(pte.get_addr());
  }

  // Standard 4KB mapping
  bool map_page(uint64_t paddr, uint64_t vaddr) const {
    if (m_pml4t == nullptr)
      return false;

    auto offsets = get_offsets(vaddr);
    auto pdpt = get_or_create_table(m_pml4t, offsets.pml4_idx);
    auto pdt = get_or_create_table(pdpt, offsets.pdpt_idx);
    auto pt = get_or_create_table(pdt, offsets.pdt_idx);

    auto &pte = pt->get(offsets.pt_idx);
    pte.set_raw(paddr);
    pte.set_present();
    pte.set_rw();
    return true;
  }

  bool map_page_2mb(uint64_t paddr, uint64_t vaddr) const {
    auto offsets = get_offsets(vaddr);
    auto pdpt = get_or_create_table(m_pml4t, offsets.pml4_idx);
    auto pdt = get_or_create_table(pdpt, offsets.pdpt_idx);

    auto &pdte = reinterpret_cast<PDTE &>(pdpt->get(offsets.pdt_idx));
    pdte.pde_2mb.PT = paddr >> 21;
    pdte.pde_2mb.P = 1;
    pdte.pde_2mb.RW = 1;
    pdte.pde_2mb.PS = 1;
    return true;
  }

  bool map_page_1gb(uint64_t paddr, uint64_t vaddr) const {
    auto offsets = get_offsets(vaddr);
    auto pdpt = get_or_create_table(m_pml4t, offsets.pml4_idx);
    auto &pdpte = reinterpret_cast<PDPTE &>(pdpt->get(offsets.pdpt_idx));
    pdpte.pdpe_1gb.P = 1;
    pdpte.pdpe_1gb.RW = 1;
    pdpte.pdpe_1gb.PS = 1; // 1GB mapping
    pdpte.pdpe_1gb.PDT = paddr >> 30;
    return true;
  }
private:
  struct page_offsets {
    uint16_t pml4_idx;
    uint16_t pdpt_idx;
    uint16_t pdt_idx;
    uint16_t pt_idx;
  };

  page_offsets get_offsets(uint64_t addr) const {
    return page_offsets{
        static_cast<uint16_t>((addr >> 39) & 0x1FF),
        static_cast<uint16_t>((addr >> 30) & 0x1FF),
        static_cast<uint16_t>((addr >> 21) & 0x1FF),
        static_cast<uint16_t>((addr >> 12) & 0x1FF),
    };
  }

  PageTable *m_pml4t{nullptr};
  Allocator m_allocator;
};
