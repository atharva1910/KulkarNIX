#pragma once
#include "SerialPort.h"
#include "Slice.h"
#include <stdarg.h>

class Logger {
  private:
    inline static const char digits[] = "0123456789abcdef";
    inline static char m_buffer[1024];
    inline static uint32_t m_idx{0};

    Slice<char> buf{m_buffer, sizeof(m_buffer)};
    SerialPort sp;

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

    // TODO
    void write(const char c) { m_buffer[m_idx++] = c; }

    void write(const char* str) {
        while (*str != '\0') {
            m_buffer[m_idx++] = *str;
        }
    }

    void write(const bool b) {
        if (b)
            write("true");
        else
            write("false");
    }

    void write(const int64_t i) {
    }

    void write(const uint64_t i) {

    }

    template <typename T> void write(T x) {
        if constexpr (T(-1) < T(0)) {
            write(static_cast<int64_t>(x));
        } else {
            write(static_cast<uint64_t>(x));
        }
    }

  public:
    Logger() {}

    template <typename First, typename... Rest>
    void fprint(const char* str, const First& first, const Rest&... rest) {
        while (*str != '\0') {
            if (*str == '{' and *(str + 1) == '}') {
                write(first);
                str += 2;
                fprint(str, rest...);
            } else
                write(*str);
        }
    }

    template <typename... Args> void print(const Args&... args) {
        (write(args), ...);
    }
};
