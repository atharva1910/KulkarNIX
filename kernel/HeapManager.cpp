#include "HeapManager.h"
#include "List.h"
#include "Logger.h"
#include "PMemManager.h"

class HeapManager {
  private:
    struct MetaData {
        size_t size;
        Node links;
    };
    List m_free_list;
    size_t m_meta_data_size;

  public:
    HeapManager() = default;

    bool init() {
        m_meta_data_size = OFFSET_OF(&MetaData::links);
        return true;
    }

    bool is_chopable(MetaData* block, size_t req_size) {
        auto rem = block->size - req_size;
        Logger::printf("is_chopable {} - {} = {}", block->size, req_size, rem);
        if (rem <= sizeof(MetaData))
            return false;
        return true;
    }

    void chop_block(MetaData* block, size_t req_size) {
        auto rem = block->size - req_size;
        Logger::printf("Chopping block into {} and {}", rem, req_size);
        auto new_block =
            reinterpret_cast<MetaData*>(reinterpret_cast<uint8_t*>(&block->links) + req_size);
        new_block->size = rem - m_meta_data_size;
        m_free_list.insert(new_block->links);
        Logger::printf("Inserting new block {}", &new_block->links);
        block->size = req_size;
    }

    MetaData* check_free_list(const size_t size) {
        if (m_free_list.is_empty())
            return nullptr;

        auto iter = m_free_list.iter();
        for(Node *node = iter.next(); node != nullptr; node = iter.next()) {
            auto md = CONTAINER_OF(node, &MetaData::links);
            if (md->size >= size)
                return md;
        }

        return nullptr;
    }

    void* alloc(size_t size) {
        MetaData* meta_data = check_free_list(size);
        if (!meta_data) {
            auto num_pages = CEILING(size, static_cast<size_t>(PAGE_SIZE));
            Logger::print("Memory not found in free list ", "Allocating num pages ", num_pages);
            meta_data = reinterpret_cast<MetaData*>(PMM::alloc_pages(num_pages));
            if (meta_data == nullptr) {
                Logger::print("PMM returned nullptr");
                return nullptr;
            } else {
                Logger::print("PMM returned ", meta_data);
            }

            meta_data->size = (num_pages << 12) - m_meta_data_size;
            meta_data->links.next = nullptr;
            meta_data->links.prev = nullptr;
        } else {
            Logger::print("Memory found in free list", reinterpret_cast<uintptr_t>(meta_data));
            m_free_list.remove_node(meta_data->links);
        }

        if (is_chopable(meta_data, size)) {
            Logger::print("Block is choppable");
            chop_block(meta_data, size);
        }
        return reinterpret_cast<void*>(&meta_data->links);
    }
};

static HeapManager g_HeapManager;

namespace Heap {
    bool init() {
        return g_HeapManager.init();
    }

    void* alloc(size_t size) {
        return g_HeapManager.alloc(size);
    }

    void free(void* p) {
    }
};
