#pragma once
#include "KernelArgs.h"
#include "KulkarNIX.h"
#include "Logger.h"
#include "UEFI/efi.h"
#include "Slice.h"
#include "bitops.h"
#include "Assert.h"

class PMemManager {
  private:
    Slice<uint8_t> m_bitmap{nullptr, 0};
    PA m_bitmap_paddr{0};
    VA m_bitmap_vaddr{PA(0)};
    Logger& m_logger;
    uint64_t m_max_paddr{0};
    uint64_t m_min_paddr{0};

    void mark_pages_alloc(PA addr, uint64_t num_pages) {
        assert(m_logger, !addr.is_page_aligned(), "Addr ", addr,
               "not page aligned");

        for (uint64_t i = 0; i < num_pages; i++) {
            addr + (i * PAGE_SIZE);
            auto byte_offset = addr.get_raw() >> 3;
            auto bit_offset = addr.get_raw() & 0x7;
            BitOps::set_bit(m_bitmap[byte_offset], bit_offset);
        }
    }

    void mark_pages_free(PA addr, uint64_t num_pages) {
        assert(m_logger, !addr.is_page_aligned(), "Addr ", addr,
               "not page aligned");

        for (uint64_t i = 0; i < num_pages; i++) {
            addr + (i * PAGE_SIZE);
            auto byte_offset = addr.get_raw() >> 3;
            auto bit_offset = addr.get_raw() & 0x7;
            BitOps::clear_bit(m_bitmap[byte_offset], bit_offset);
        }
    }

  public:
    PMemManager(Logger& logger)
        : m_logger(logger) {}

    bool init(const KernelArgs* kernelArgs) {
        auto& mm_info = kernelArgs->mm_info;
        m_max_paddr = mm_info.max_paddr;
        m_min_paddr = mm_info.min_paddr;
        auto total_bits = mm_info.total_memory >> PAGE_SIZE_SHIFT;
        auto total_bytes = total_bits >> 3;
        auto total_pages = total_bytes >> PAGE_SIZE_SHIFT;
        EFI_MEMORY_DESCRIPTOR* pdesc = nullptr;

        for (int i = 0; i < mm_info.num_desc; i++) {
            auto desc_addr = mm_info.mm + (i * mm_info.dsize);
            pdesc = reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(desc_addr.get_raw());

            if (pdesc->Type != EfiConventionalMemory &&
                pdesc->Type != EfiLoaderData && pdesc->Type != EfiLoaderCode &&
                pdesc->Type != EfiBootServicesData &&
                pdesc->Type != EfiBootServicesCode)
                continue;

            if (pdesc->NumberOfPages < total_pages)
                continue;

            m_logger.printf(
                "Selecting: {} required pages: {}. Desc Pages {}. PhyStart: {}",
                i, total_pages, pdesc->NumberOfPages, pdesc->PhysicalStart);

            m_bitmap_paddr = PA(pdesc->PhysicalStart);
            m_bitmap_vaddr = VA(m_bitmap_paddr);
            m_bitmap = Slice<uint8_t>(m_bitmap_vaddr, total_bytes);
            break;
        }

        assert(m_logger, m_bitmap.size() != 0, "Failed find mem for PhyMemoryMap");

        m_bitmap.fill(0);
        auto& kinfo = kernelArgs->k_info;

        m_logger.print("Marking the bitmap buffer :  ",
                       reinterpret_cast<uint64_t>(m_bitmap.get_buf()),
                       " size : ", total_pages);

        mark_pages_alloc(m_bitmap_paddr, total_pages);

        m_logger.print("Marking the kernel :  ", kernelArgs->k_info.kernelPAddr,
                       " size : ", kernelArgs->k_info.kernelPages);

        mark_pages_alloc(kernelArgs->k_info.kernelPAddr,
                         kernelArgs->k_info.kernelPages);

        return true;
    }
};
