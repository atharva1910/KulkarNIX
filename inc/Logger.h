#pragma once
#include "serial_port.h"
#include "slice.h"
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

    size_t itoa(int number, Slice<char> s, int radix) {
    size_t idx = 0;
    number = reverse(number, radix);
    do {
      int tmp = number % radix;
      number = number / radix;
      if (tmp < 10)
        s[idx++] = tmp + '0';
      else
        s[idx++] = tmp + 'a' - 10;
    } while (number);
    return idx;
  }

  bool vsprintf(Slice<char> &s, const char *str, va_list args) {
    if (s.size() == 0 || str == nullptr)
      return false;

    const char *itr = str;
    size_t idx = 0;

    while (*itr != '\0' && idx < s.size()) {
      /* keep copying until we hit a '%' */
      if (*itr != '%') {
        s[idx++] = *itr;
        itr++;
        continue;
      }

      /* We hit a % lets get the "type" of the argument to be printed */
      switch (*(++itr)) {
      case 'd': {
          idx += itoa(va_arg(args, int), s.sub_slice(idx, s.size()), 10);
      } break;

      case 'x': {
          idx += itoa(va_arg(args, int), s.sub_slice(idx, s.size()), 16);
      } break;

      default: {
        return false;
      }
      }
    }

    if (idx < s.size() && s.size() - idx >= 2) {
        s[idx++] = '\n';
        s[idx] = '\0';
    } else {
        return false;
    }
    return true;
  }

public:
  void print(const char *str, ...) {
    va_list valist;
    auto buf = Slice(m_buffer, sizeof(m_buffer));
    va_start(valist, str);
    if (vsprintf(buf, str, valist)) {
      sp.write(m_buffer);
    }
    va_end(valist);
  }
};
