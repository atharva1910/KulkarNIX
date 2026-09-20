#pragma once
#include <Slice.h>

class String {
  private:
    Slice<char> s;
    size_t idx{0};

  public:
    String(char* str, size_t size) : s(Slice<char>(str, size)) { idx = 0; }

    bool append(const char& c) {
        if (idx >= s.size()) {
            return false;
        }

        s[idx++] = c;
        return true;
    }

    bool append(const char* c) {
        bool ret = true;
        while(*c != '\0' && ret) {
            ret = append(*c);
            c++;
        }
        return ret;
    }

    bool write(const bool b) {
        if (b)
            return append("true");
        else
            return append("false");
    }

    const char* c_str() {
        return s.get_buf();
    }
};
