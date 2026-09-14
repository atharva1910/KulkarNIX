#pragma once
#include "hal.h"

class SerialPort {
public:
    static constexpr uint16_t COM1 = 0x3F8;
    static constexpr uint16_t RECV_BUF =  COM1 + 0;
    static constexpr uint16_t TRANS_BUF =  COM1 + 0;
    static constexpr uint16_t INTR_REG =  COM1 + 1;
    static constexpr uint16_t BAUD_LSB =  COM1 + 0;
    static constexpr uint16_t BAUD_MSB =  COM1 + 1;
    static constexpr uint16_t INTR_ID =  COM1 + 2;
    static constexpr uint16_t FIFO_CTRL_REG =  COM1 + 2;
    static constexpr uint16_t LINE_CTRL_REG =  COM1 + 3;
    static constexpr uint16_t MODEM_CTRL_REG =  COM1 + 4;
    static constexpr uint16_t LINE_STATUS_REG =  COM1 + 5;
    static constexpr uint16_t MODEM_STATUS_REG =  COM1 + 6;
    static constexpr uint16_t SCRATCH_REG = COM1 + 7;

    SerialPort() { init(); }

    void init() {
      HAL::outb(INTR_REG, 0x0);
      HAL::outb(LINE_CTRL_REG, 0x80);
      HAL::outb(BAUD_LSB, 0x3);
      HAL::outb(BAUD_MSB, 0x0);
      HAL::outb(FIFO_CTRL_REG, 0xC7);
      HAL::outb(MODEM_CTRL_REG, 0x0F);
    }

    void write(const char *str) {
      const char *c = str;
      while (*c != '\0') {
          while ((HAL::inb(LINE_STATUS_REG) & 0x20) == 0);
          HAL::outb(TRANS_BUF, *c);
          c++;
      }
    }
};
