#pragma once
#include "SerialPort.h"
#include "Slice.h"
#include <stdarg.h>

class Logger {
  private:
    inline static const char digits[] = "0123456789ABCDEF";
    inline static char m_raw_buffer[1024];
    inline static uint32_t m_idx{0};
    Slice<char> m_buf{m_raw_buffer, sizeof(m_raw_buffer)};
    SerialPort sp;

    void itoa(uint64_t number, int radix) {
        size_t idx = 0;
        char b[21] = {0};
        do {
            int tmp = number % radix;
            number = number / radix;
            b[idx++] = digits[tmp];
        } while (number);

        while (idx != 0) {
            m_buf[m_idx++] = b[--idx];
        }
    }

    void write(const char c) { m_buf[m_idx++] = c; }

    void write(const char* str) {
        while (*str != '\0') {
            m_buf[m_idx++] = *str;
            str++;
        }
    }

    void write(const bool b) {
        if (b)
            write("true");
        else
            write("false");
    }

    void write(int64_t i) {
        bool is_negative = i < 0 ? true : false;
        if (is_negative) {
            m_buf[m_idx++] = '-';
            itoa(static_cast<uint64_t>(-i), 16);
        } else {
            itoa(i, 16);
        }
    }

    void write(uint64_t i) { itoa(i, 16); }

    template <typename T> void write(T x) {
        if constexpr (T(-1) < T(0)) {
            write(static_cast<int64_t>(x));
        } else {
            write(static_cast<uint64_t>(x));
        }
    }

    void print_impl(const char* str) {
        while (*str != '\0') {
            write(*str);
            str++;
        }
    }

    template <typename First, typename... Rest>
    void print_impl(const char* str, First& first, const Rest&... rest) {
        while (*str != '\0') {
            if (*str == '{' and *(str + 1) == '}') {
                write(first);
                str += 2;
                return print_impl(str, rest...);
            } else {
                write(*str);
                str++;
            }
        }
    }

  public:
    Logger() {}

    // void printf(const char* str, const Args&... args) {
    template <typename... Args>
    void print(const char* str, const Args&... args) {
        m_idx = 0;
        print_impl(str, args...);
        write('\n');
        write('\0');
        sp.write(m_buf.get_buf());
    }

    template <typename... Args> void print(const Args&... args) {
        m_idx = 0;
        (write(args), ...);
        write('\n');
        write('\0');
        sp.write(m_buf.get_buf());
    }
};
