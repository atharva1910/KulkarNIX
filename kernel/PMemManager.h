#pragma once
#include "KulkarNIX.h"
#include "KernelArgs.h"
#include "Logger.h"
#include "UEFI/efi.h"

class PMemManager {
  private:
    uint64_t m_dsize;
    uint64_t m_size;
    uint64_t m_total_memory;
    uint32_t m_num_desc;
    uint8_t* mm;
    Logger& m_logger;

  public:
    PMemManager(Logger& logger, MemMapInfo& mm_info) : m_logger(logger) {
        m_dsize = mm_info.dsize;
        m_size = mm_info.size;
        m_num_desc = mm_info.num_desc;
        m_total_memory = mm_info.total_memory;
        mm = mm_info.mm;

        auto total_bits = m_total_memory >> PAGE_SIZE_SHIFT;
        auto total_bytes = total_bits >> 3;
        auto total_pages = total_bytes >> PAGE_SIZE_SHIFT;

        for (int i = 0; i < m_num_desc; i++) {
            EFI_MEMORY_DESCRIPTOR *pdesc =
                reinterpret_cast<EFI_MEMORY_DESCRIPTOR *>(mm + (i * m_dsize));

            if (pdesc->Type != EfiConventionalMemory &&
                pdesc->Type != EfiLoaderData &&
                pdesc->Type != EfiLoaderCode &&
                pdesc->Type != EfiBootServicesData &&
                pdesc->Type != EfiBootServicesCode) {
                continue;
            }

            if (pdesc->NumberOfPages >= total_pages) {
                m_logger.print("Selecting: %d required pages: %d. Desc Pages %d. PhyStart: 0x%x", i, total_pages, pdesc->NumberOfPages, pdesc->PhysicalStart);
            }
        }

        m_logger.print("Initialised MemoryMap. Total Memory %x", m_total_memory);
    }
};
