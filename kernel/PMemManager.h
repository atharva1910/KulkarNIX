#pragma once
#include "KernelArgs.h"
#include "KulkarNIX.h"
#include "Logger.h"
#include "Paging.h"
#include "UEFI/efi.h"
#include "Slice.h"
#include "bitops.h"
#include "Assert.h"
class PMemManager {
  private:
    Slice<uint8_t> m_bitmap{nullptr, 0};
    Logger& m_logger;

    void mark_pages_alloc(uint64_t addr, uint64_t num_pages) {
        // assert(m_logger, (addr & (PAGE_SIZE - 1)) == 0, "Addr 0x%x not page
        // aligned", addr);
        for (uint64_t i = 0; i < num_pages; i++) {
            auto page_addr = addr + (i * PAGE_SIZE);
            auto byte_offset = page_addr >> 3;
            auto bit_offset = page_addr & 0x7;
            BitOps::set_bit(m_bitmap[byte_offset], bit_offset);
        }
    }

    void mark_pages_free(uint64_t addr, uint64_t num_pages) {
        for (uint64_t i = 0; i < num_pages; i++) {
            auto page_addr = addr + (i * PAGE_SIZE);
            auto byte_offset = page_addr >> 3;
            auto bit_offset = page_addr & 0x7;
            BitOps::clear_bit(m_bitmap[byte_offset], bit_offset);
        }
    }

  public:
    PMemManager(Logger& logger)
        : m_logger(logger) {}

    bool init(const KernelArgs* kernelArgs) {
        auto& mm_info = kernelArgs->mm_info;
        //        m_logger.fprint("Initialising MemoryMap. Total Memory {}", mm_info.total_memory);

        auto total_bits = mm_info.total_memory >> PAGE_SIZE_SHIFT;
        auto total_bytes = total_bits >> 3;
        auto total_pages = total_bytes >> PAGE_SIZE_SHIFT;
        EFI_MEMORY_DESCRIPTOR* pdesc = nullptr;

        for (int i = 0; i < mm_info.num_desc; i++) {
            pdesc =
                reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(mm_info.mm + (i * mm_info.dsize));

            if (pdesc->Type != EfiConventionalMemory &&
                pdesc->Type != EfiLoaderData && pdesc->Type != EfiLoaderCode &&
                pdesc->Type != EfiBootServicesData &&
                pdesc->Type != EfiBootServicesCode)
                continue;

            if (pdesc->NumberOfPages < total_pages)
                continue;

            m_logger.print(
                "Selecting: {} required pages: {}. Desc Pages {}. PhyStart: {}",
                i, total_pages, pdesc->NumberOfPages, pdesc->PhysicalStart);
            break;
        }

        auto bitmap_pointer =
            reinterpret_cast<uint8_t *>(PA2VA(pdesc->PhysicalStart));

        m_bitmap = Slice<uint8_t>(bitmap_pointer, total_bytes);
        //m_logger.print("Initialised MemoryMap");

        if (m_bitmap.size() == 0) {
            m_logger.print("Failed to init PhyMemoryMap");
            return false;
        }

        assert(m_logger, false, "Failed to init PhyMemoryMap");
        m_bitmap.fill(0);
        auto& kinfo = kernelArgs->k_info;

        return true;
    }
};
