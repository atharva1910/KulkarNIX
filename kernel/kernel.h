#pragma once
#include "Logger.h"
#include "kernel_args.h"
#include "KulkarNIX.h"
#include "memory_map.h"

class KernelContext {
private:
  KernelContext();
  Logger m_logger;
  KernelArgs *m_args;
  MemoryMap m_mm;
public:
  static KernelContext &get() {
      static KernelContext m_kcontext;
      return m_kcontext;
  }

  static void init(void *args) {
    get().m_args = reinterpret_cast<KernelArgs *>(PA2VA<void *>(args));
  }

  static Logger &get_logger() {
      return get().m_logger;
  }
};
