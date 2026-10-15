#include "Logger.h"
#include "KulkarNIX.h"
#include "Slice.h"

void itoa(uint64_t number, int radix, Slice<char>& buffer) {
    size_t idx = 0;
    char b[21] = {0};
    static const char digits[] = "0123456789ABCDEF";
    do {
        int tmp = number % radix;
        number = number / radix;
        b[idx++] = digits[tmp];
    } while (number);

    while (idx != 0) {
        buffer.push(b[--idx]);
    }
}

void format(const char c, Slice<char>& buffer) {
    buffer.push(c);
}

void format(uint64_t i, Slice<char>& buffer) {
    itoa(i, 16, buffer);
}

void format(VA x, Slice<char>& buffer) {
    format(x.get_raw(), buffer);
}

void format(PA x, Slice<char>& buffer) {
    format(x.get_raw(), buffer);
}

void format(const char* str, Slice<char>& buffer) {
    while (*str != '\0') {
        buffer.push(*str);
        str++;
    }
}

void format(const bool b, Slice<char>& buffer) {
    if (b)
        format("true", buffer);
    else
        format("false", buffer);
}

void format(int64_t i, Slice<char>& buffer) {
    bool is_negative = i < 0 ? true : false;
    if (is_negative) {
        buffer.push('-');
        itoa(static_cast<uint64_t>(-i), 16, buffer);
    } else {
        itoa(i, 16, buffer);
    }
}

void printf_impl(Slice<char>& buffer, const char* str) {
    while (*str != '\0') {
        format(*str, buffer);
        str++;
    }
}
