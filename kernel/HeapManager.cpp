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

  public:
    HeapManager() = default;

    bool init() { return false; }

    void chop_block(MetaData* block) {}

    MetaData* check_free_list(const size_t size) {
        if (m_free_list.is_empty())
            return nullptr;

        auto iter = m_free_list.iter();
        for(Node *node =iter.next(); node != nullptr; node = iter.next()) {
            auto md = container_of(node, &MetaData::links);
            if (md->size >= size)
                return md;
            node = iter.next();
        }

        return nullptr;
    }

    void* alloc(size_t size) {
        MetaData* meta_data = check_free_list(size);

        if (!meta_data) {
            auto num_pages = CEILING(size, static_cast<size_t>(PAGE_SIZE));
            Logger::print("Memory not found in free list ", "Allocating num pages ", num_pages);
            auto p = reinterpret_cast<MetaData*>(PMM::alloc_pages(num_pages));
            if (p == nullptr) {
                Logger::print("PMM returned nullptr");
                return nullptr;
            } else {
                Logger::print("PMM returned ", p);
            }

            p->size = num_pages << 12;
            p->links.next = nullptr;
            p->links.prev = nullptr;
            return reinterpret_cast<void *>(&p->links);
        }

        Logger::print("Memory found in free list", reinterpret_cast<uintptr_t>(meta_data));
        m_free_list.remove_node(meta_data->links);
        return reinterpret_cast<void *>(&meta_data->links);
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
