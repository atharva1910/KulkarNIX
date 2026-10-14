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
void assert(bool exp, Args&... args) {
    if (exp)
        return;
    Logger::print("[ASSERT START]");
    Logger::print(args...);
    Logger::print("[ASSERT END]");
    HAL::hlt();
}
