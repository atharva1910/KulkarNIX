#pragma once
#include "Slice.h"
#include "SerialPort.h"

void format(const char c, Slice<char>& buffer);
void format(const char*, Slice<char>& buffer);
void format(uint64_t i, Slice<char>& buffer);
void format(VA x, Slice<char>& buffer);
void format(PA x, Slice<char>& buffer);
void format(const char* str, Slice<char>& buffer);
void format(const bool b, Slice<char>& buffer);
void format(int64_t i, Slice<char>& buffer);
template <typename T> void format(T* p, Slice<char>& buffer) {
    format(reinterpret_cast<uintptr_t>(p), buffer);
}
template <typename T>
void format(T x, Slice<char>& buffer) {
    if constexpr (T(-1) < T(0)) {
        format(static_cast<int64_t>(x), buffer);
    } else {
        format(static_cast<uint64_t>(x), buffer);
    }
}

void printf_impl(Slice<char>& buffer, const char* str);

template <typename First, typename... Rest>
void printf_impl(Slice<char>& buffer, const char* str, First& first, const Rest&... rest) {
    while (*str != '\0') {
        if (*str == '{' and *(str + 1) == '}') {
            format(first, buffer);
            str += 2;
            return printf_impl(buffer, str, rest...);
        } else {
            format(*str, buffer);
            str++;
        }
    }
}

namespace Logger {
    template <typename... Args> void print(const Args&... args) {
        char m_raw_buffer[1024];
        Slice<char> m_buf{m_raw_buffer, sizeof(m_raw_buffer)};
        (format(args, m_buf), ...);
        format('\n', m_buf);
        format('\0', m_buf);
        SerialPort::write(m_buf.get_buf());
    }


    template <typename... Args> void printf(const char* str, const Args&... args) {
        char m_raw_buffer[1024];
        Slice<char> m_buf{m_raw_buffer, sizeof(m_raw_buffer)};
        printf_impl(m_buf, str, args...);
        format('\n', m_buf);
        format('\0', m_buf);
        SerialPort::write(m_buf.get_buf());
    }
}
