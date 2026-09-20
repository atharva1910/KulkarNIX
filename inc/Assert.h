#pragma once
#include "Logger.h"
#include "HAL.h"

void dump_regs() {
}
/*
  called as
  assert(logger, false, "Print this");
  assert(logger, false, "Print this {} ", 42);
  assert(logger, false);
 */
template <typename... Args>
void assert(Logger& logger, bool exp, Args&... args) {
    if (exp)
        return;
    logger.print("[ASSERT START]");
    logger.print(args...);
    logger.print("[ASSERT END]");
    HAL::hlt();
}
