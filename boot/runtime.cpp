// runtime.cpp - Minimal Freestanding C++ Support
extern "C" {

void* memcpy(void* dest, const void* src, size_t n) {
    auto* d = static_cast<unsigned char*>(dest);
    const auto* s = static_cast<const unsigned char*>(src);
    while (n--) *d++ = *s++;
    return dest;
}

void* memset(void* dest, int c, size_t n) {
    auto* d = static_cast<unsigned char*>(dest);
    while (n--) *d++ = static_cast<unsigned char>(c);
    return dest;
}

void* memmove(void* dest, const void* src, size_t n) {
    auto* d = static_cast<unsigned char*>(dest);
    const auto* s = static_cast<const unsigned char*>(src);
    if (d < s) {
        while (n--) *d++ = *s++;
    } else {
        d += n; s += n;
        while (n--) *--d = *--s;
    }
    return dest;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const auto* p1 = static_cast<const unsigned char*>(s1);
    const auto* p2 = static_cast<const unsigned char*>(s2);
    while (n--) {
        if (*p1 != *p2) return *p1 - *p2;
        p1++; p2++;
    }
    return 0;
}

} // extern "C"
