#pragma once
#include "efi.h"
#include "boot_context.h"
#include "paging.h"

class MemoryMap {
public:
    MemoryMap(BootCtx *b) : ctx(b) { refresh(); }

    void refresh() {
        if (mm) {
            ctx->boot_services()->FreePool(mm);
        }

        ctx->boot_services()->GetMemoryMap(&m_size, nullptr, &m_key, &m_dsize, &m_dver);

        m_size += PAGE_SIZE;
        m_num_desc = m_size / m_dsize;

        if (EFI_ERROR(ctx->boot_services()->AllocatePool(EfiBootServicesData, m_size, (void **)&mm)))
            ctx->halt(L"FAILED TO ALLOCATE MEM FOR MEMORY MAP");

        if (EFI_ERROR(ctx->boot_services()->GetMemoryMap(&m_size, (EFI_MEMORY_DESCRIPTOR *)mm,
                                                         &m_key, &m_dsize,
                                                         &m_dver)))
            ctx->halt(L"FAILED TO GET MEMORY MAP");


        for (int i = 0; i < m_num_desc; i++) {
            EFI_MEMORY_DESCRIPTOR *pdesc =
                reinterpret_cast<EFI_MEMORY_DESCRIPTOR *>(mm + (i * m_dsize));

            if (pdesc->Type == EfiConventionalMemory ||
                pdesc->Type == EfiLoaderData || pdesc->Type == EfiLoaderCode ||
                pdesc->Type == EfiBootServicesData || pdesc->Type == EfiBootServicesCode)
                m_total_mem += pdesc->NumberOfPages << 12;

            if (pdesc->PhysicalStart < m_min_paddr)
                m_min_paddr = pdesc->PhysicalStart;

            if ((pdesc->PhysicalStart + (pdesc->NumberOfPages << 12)) > m_max_paddr)
                m_max_paddr = (pdesc->PhysicalStart + (pdesc->NumberOfPages << 12));

            if (pdesc->VirtualStart < m_min_vaddr)
                m_min_vaddr = pdesc->VirtualStart;

            if ((pdesc->VirtualStart + (pdesc->NumberOfPages << 12)) > m_max_vaddr)
                m_max_vaddr = (pdesc->VirtualStart + (pdesc->NumberOfPages << 12));
        }
    }

    ~MemoryMap() {
      if (mm) {
          ctx->boot_services()->FreePool(mm);
      }
    }

    BootCtx *ctx = nullptr;
    uint64_t m_key = 0;
    uint64_t m_dsize = 0;
    uint64_t m_size = 0;
    uint32_t m_dver = 0;
    uint32_t m_num_desc = 0;
    uint8_t *mm = nullptr;
    uint64_t m_min_paddr = -1, m_max_paddr = 0;
    uint64_t m_min_vaddr = -1, m_max_vaddr = 0;
    UINT64 m_total_mem = 0;

};
