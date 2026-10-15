#pragma once
#include "Slice.h"
#include "SerialPort.h"

void format(const char*, Slice<char>& buffer);
void format(uint64_t i, Slice<char>& buffer);
void format(VA x, Slice<char>& buffer);
void format(PA x, Slice<char>& buffer);
void format(const char* str, Slice<char>& buffer);
void format(const bool b, Slice<char>& buffer);
void format(int64_t i, Slice<char>& buffer);
template <typename T> void format(T x, Slice<char>& buffer);

void printf_impl(const char* str, Slice<char>& buffer);

template <typename First, typename... Rest>
void printf_impl(const char* str, First& first, const Rest&... rest, Slice<char>& buffer);

namespace Logger {
    template <typename... Args> void print(const Args&... args) {
        char m_raw_buffer[1024];
        Slice<char> m_buf{m_raw_buffer, sizeof(m_raw_buffer)};
        (format(args, m_buf), ...);
        format("\n\0", m_buf);
        SerialPort::write(m_buf.get_buf());
    }


    template <typename... Args> void printf(const char* str, const Args&... args) {
        char m_raw_buffer[1024];
        Slice<char> m_buf{m_raw_buffer, sizeof(m_raw_buffer)};
        printf_impl(str, args..., m_buf);
        format("\n\0", m_buf);
        SerialPort::write(m_buf.get_buf());
    }
}
