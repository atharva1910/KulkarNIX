#pragma once
#include <stdint.h>

namespace BitOps {

void set_bit(uint8_t& byte, const uint8_t bit_pos) {
    byte |= (1 << bit_pos);
}

void clear_bit(uint8_t& byte, const uint8_t bit_pos) {
    byte &= ~(1 << bit_pos);
}

}
