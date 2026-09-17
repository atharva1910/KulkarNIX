#pragma once
#include "KernelArgs.h"
#include "KulkarNIX.h"
#include "Logger.h"
#include "Paging.h"
#include "UEFI/efi.h"
#include "Slice.h"
#include "bitops.h"

class PMemManager {
  private:
    Slice<uint8_t> m_bitmap{nullptr, 0};
    Logger& m_logger;

    void mark_page_alloc(uint64_t addr) {
        //assert(m_logger, (addr & (PAGE_SIZE - 1)) == 0, "Addr 0x%x not page aligned", addr);
        auto byte_offset = addr >> 3;
        auto bit_offset = addr & 0x7;
        BitOps::set_bit(m_bitmap[byte_offset], bit_offset);
    }

    void mark_page_free(uint64_t addr) {
        auto byte_offset = addr >> 3;
        auto bit_offset = addr & 0x7;
        BitOps::clear_bit(m_bitmap[byte_offset], bit_offset);
    }

  public:
    PMemManager(Logger& logger)
        : m_logger(logger) {}

    bool init(const KernelArgs* kernelArgs) {
        auto& mm_info = kernelArgs->mm_info;
        m_logger.print("Initialising MemoryMap. Total Memory %x",
                       mm_info.total_memory);

        auto total_bits = mm_info.total_memory >> PAGE_SIZE_SHIFT;
        auto total_bytes = total_bits >> 3;
        auto total_pages = total_bytes >> PAGE_SIZE_SHIFT;

        for (int i = 0; i < mm_info.num_desc; i++) {
            EFI_MEMORY_DESCRIPTOR* pdesc =
                reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(mm_info.mm + (i * mm_info.dsize));

            if (pdesc->Type != EfiConventionalMemory &&
                pdesc->Type != EfiLoaderData && pdesc->Type != EfiLoaderCode &&
                pdesc->Type != EfiBootServicesData &&
                pdesc->Type != EfiBootServicesCode)
                continue;

            if (pdesc->NumberOfPages < total_pages)
                continue;

            m_logger.print("Selecting: %d required pages: %d. Desc Pages "
                           "%d. PhyStart: 0x%x",
                           i, total_pages, pdesc->NumberOfPages,
                           pdesc->PhysicalStart);

            auto bitmap_pointer =
                reinterpret_cast<uint8_t *>(PA2VA(pdesc->PhysicalStart));

            m_bitmap = Slice<uint8_t>(bitmap_pointer, total_bytes);
            m_logger.print("Initialised MemoryMap");
            break;
        }

        if (m_bitmap.size() == 0) {
            m_logger.print("Failed to init PhyMemoryMap");
            return false;
        }

        m_bitmap.fill(0);
        auto& kinfo = kernelArgs->k_info;
        mark_page_alloc(1);

        return true;
    }
};
