#include "PMemManager.h"
#include "KernelArgs.h"
#include "KulkarNIX.h"
#include "Slice.h"
#include "UEFI/efi.h"
#include "bitops.h"
#include "Assert.h"

class PMemManager {
  private:
    Slice<uint8_t> m_bitmap{nullptr, 0};
    size_t m_num_pages;
    PA m_bitmap_paddr{0};
    VA m_bitmap_vaddr{PA(0)};
    PA m_max_paddr{0};
    PA m_min_paddr{0};

    void mark_pages_alloc(const PA& addr, uint64_t num_pages) {
        assert(addr.is_page_aligned(), "Addr ", addr, " not page aligned");
        auto raw_addr = addr.get_raw();
        for (uint64_t i = 0; i < num_pages; i++) {
            raw_addr += (i * PAGE_SIZE);
            auto byte_offset = raw_addr >> 3;
            auto bit_offset = raw_addr & 0x7;
            BitOps::set_bit(m_bitmap[byte_offset], bit_offset);
        }
    }

    void mark_pages_free(PA addr, uint64_t num_pages) {
        assert(addr.is_page_aligned(), "Addr ", addr, " not page aligned");
        auto raw_addr = addr.get_raw();
        for (uint64_t i = 0; i < num_pages; i++) {
            raw_addr += (i * PAGE_SIZE);
            auto byte_offset = raw_addr >> 3;
            auto bit_offset = raw_addr & 0x7;
            BitOps::clear_bit(m_bitmap[byte_offset], bit_offset);
        }
    }

    bool is_page_free(size_t page_num) {
        auto byte_offset = page_num >> 3;
        auto bit_offset = page_num & 0x7;
        return BitOps::get_bit(m_bitmap[byte_offset], bit_offset);
    }

  public:
    constexpr PMemManager() = default;

    bool init(const KernelArgs* kernelArgs) {
        auto& mm_info = kernelArgs->mm_info;
        m_max_paddr = mm_info.max_paddr;
        m_min_paddr = mm_info.min_paddr;
        m_num_pages = mm_info.total_memory >> PAGE_SIZE_SHIFT;
        auto total_bytes = m_num_pages >> 3;
        auto total_pages = total_bytes >> PAGE_SIZE_SHIFT;
        EFI_MEMORY_DESCRIPTOR* pdesc = nullptr;

        for (int i = 0; i < mm_info.num_desc; i++) {
            auto desc_addr = mm_info.mm + (i * mm_info.dsize);
            pdesc = reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(desc_addr.get_raw());

            if (pdesc->Type != EfiConventionalMemory && pdesc->Type != EfiLoaderData &&
                pdesc->Type != EfiLoaderCode && pdesc->Type != EfiBootServicesData &&
                pdesc->Type != EfiBootServicesCode)
                continue;

            if (pdesc->NumberOfPages < total_pages)
                continue;

            Logger::printf("Selecting: {} required pages: {}. Desc Pages {}. PhyStart: {}", i,
                           total_pages, pdesc->NumberOfPages, pdesc->PhysicalStart);

            m_bitmap_paddr = PA(pdesc->PhysicalStart);
            m_bitmap_vaddr = VA(m_bitmap_paddr);
            m_bitmap = Slice<uint8_t>(m_bitmap_vaddr, total_bytes);
            break;
        }

        assert(m_bitmap.size() != 0, "Failed to find memory for bitmap");

        m_bitmap.fill(1);

        for (int i = 0; i < mm_info.num_desc; i++) {
            auto desc_addr = mm_info.mm + (i * mm_info.dsize);
            pdesc = reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(desc_addr.get_raw());

            if (pdesc->Type != EfiConventionalMemory && pdesc->Type != EfiLoaderData &&
                pdesc->Type != EfiLoaderCode && pdesc->Type != EfiBootServicesData &&
                pdesc->Type != EfiBootServicesCode)
                continue;

            Logger::print("Making ", pdesc->PhysicalStart, " free for pages: ", pdesc->NumberOfPages);
            mark_pages_free(PA(pdesc->PhysicalStart), pdesc->NumberOfPages);
        }

        Logger::print("Making ", m_bitmap_paddr, " allocated for pages: ", total_pages);
        mark_pages_alloc(m_bitmap_paddr, total_pages);
        Logger::print("Making ", kernelArgs->k_info.kernelPAddr, " allocated for pages: ", kernelArgs->k_info.kernelPages);
        mark_pages_alloc(kernelArgs->k_info.kernelPAddr, kernelArgs->k_info.kernelPages);
        return true;
    }

    void* alloc_pages(size_t npages) {
        assert(npages != 0, "Num pages cannot be 0");
        size_t count = 0;
        size_t idx = 0;

        while (idx != m_num_pages) {
            if (!is_page_free(idx++)) {
                count = 0;
                continue;
            }
            count++;
            Logger::print("Free page found ", idx, " count ", count, " required pages ", npages);
            if (count == npages)
                break;
            else if (idx >= 10)
                break;
        }

        if (count == 0)
            return nullptr;

        auto start = idx - count;
        Logger::print("Found memory at bit ", start);
        auto pa = PA(start << 12);
        mark_pages_alloc(pa, count);
        return reinterpret_cast<void *>(VA(pa).get_raw());
    }
};

static PMemManager g_pmm;

namespace PMM {
    bool init(const KernelArgs* kernelArgs) {
        return g_pmm.init(kernelArgs);
    }

    void* alloc_pages(size_t n) {
        return g_pmm.alloc_pages(n);
    }

    void free(void* p) {
    }
}
