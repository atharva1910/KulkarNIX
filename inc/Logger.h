#pragma once
#include "serial_port.h"
#include "slice.h"
#include <stdarg.h>

class Logger {
  private:
    SerialPort sp;
    inline static char m_buffer[1024];
    inline static const char digits[] = "0123456789abcdef";

    size_t itoa(uint64_t number, Slice<char> s, int radix) {
        size_t idx = 0;
        char b[21] = {0};
        do {
            int tmp = number % radix;
            number = number / radix;
            b[idx++] = digits[tmp];
        } while (number);

        size_t i = 0;
        while (idx != 0 && i < s.size()) {
            s[i++] = b[--idx];
        }

        return i;
    }

    bool vsprintf(Slice<char>& s, const char* str, va_list args) {
        if (s.size() == 0 || str == nullptr)
            return false;

        const char* itr = str;
        size_t idx = 0;

        while (*itr != '\0' && idx < s.size()) {
            if (*itr != '%') {
                s[idx++] = *itr;
                itr++;
                continue;
            } else {
                itr++;
            }

            switch (*itr) {
            case 'd': {
                idx += itoa(va_arg(args, uint64_t), s.sub_slice(idx, s.size()), 10);
            } break;

            case 'x': {
                idx += itoa(va_arg(args, uint64_t), s.sub_slice(idx, s.size()), 16);
            } break;

            default:
                return false;
            }

            itr++;
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
    void print(const char* str, ...) {
        va_list valist;
        auto buf = Slice(m_buffer, sizeof(m_buffer));
        va_start(valist, str);
        if (vsprintf(buf, str, valist)) {
            sp.write(m_buffer);
        }
        va_end(valist);
    }
};
