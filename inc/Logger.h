#pragma once
#include "serial_port.h"
#include <stdarg.h>

class Logger {
private:
  SerialPort sp;
  inline static char m_buffer[1024];

  int reverse(int number, int radix) {
    int ret = 0;
    while (number) {
      ret = ret * radix + (number % radix);
      number = number / radix;
    }
    return ret;
  }

  char *itoa(int number, char *m_buffer, int radix) {
    number = reverse(number, radix);
    do {
      int tmp = number % radix;
      number = number / radix;
      if (tmp < 10)
        *m_buffer = tmp + '0';
      else
        *m_buffer = tmp + 'a' - 10;
      m_buffer++;
    } while (number);

    return m_buffer;
  }

  bool vsprintf(char *buf, const char *str, va_list args) {
    if (buf == nullptr || str == nullptr)
        return false;

    const char *itr = str;
    char *head = buf;

    while (*itr != '\0') {
      /* keep copying until we hit a '%' */
      if (*itr != '%') {
        *head = *itr;
        head++;
        itr++;
        continue;
      }

      /* We hit a % lets get the "type" of the argument to be printed */
      switch (*(++itr)) {
      case 'd': {
        head = itoa(va_arg(args, int), head, 10);
        if (head == nullptr)
          return false;
        itr++;
      } break;

      case 'x': {
        head = itoa(va_arg(args, int), head, 16);
        if (head == nullptr)
          return false;
        itr++;
      } break;

      default: {
        return -1;
      }
      }
    }

    *head = '\n';
    head++;
    *head = '\0';
    return true;
  }

public:
  void print(const char *str, ...) {
    va_list valist;

    va_start(valist, str);
    if (vsprintf(m_buffer, str, valist)) {
        sp.write(m_buffer);
    }
    va_end(valist);
  }
};
