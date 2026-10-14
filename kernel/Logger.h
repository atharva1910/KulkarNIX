#pragma once
#include "Slice.h"
#include "SerialPort.h"

void format(const char*, Slice<char>& buffer);

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
