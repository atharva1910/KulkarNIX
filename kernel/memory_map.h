#pragma once
#include "KernelArgs.h"
#include "Logger.h"

class MemoryMap {
 private:
    uint64_t dsize;
    uint64_t size;
    uint64_t total_memory;
    uint32_t num_desc;
    uint8_t *mm;
    Logger &m_logger;

  public:
    MemoryMap(Logger &logger, MemMapInfo &mm_info) : m_logger(logger) {
      dsize = mm_info.dsize;
      size = mm_info.size;
      num_desc = mm_info.num_desc;
      mm = mm_info.mm;
      m_logger.print("Initialised MemoryMap. %x", size);
    }
};
