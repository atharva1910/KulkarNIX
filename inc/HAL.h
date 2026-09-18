#pragma once
#include <stdint.h>

namespace HAL {
inline void outb(uint16_t port, uint8_t input) {
  asm volatile("out %%al, %%dx"
               ::"a"(input), "d"(port));
}

inline uint8_t inb(uint16_t port) {
  uint8_t ret = 0;
  asm volatile("in %%dx, %%al" : "=a"(ret) : "d"(port));
  return ret;
}
}
