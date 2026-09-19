#pragma once
#include <stddef.h>
#include <KulkarNIX.h>

template <typename T> class Slice {
  private:
    size_t m_size{0};
    T* m_buf{nullptr};

  public:
    Slice(T* buf, size_t buf_size) : m_buf(buf), m_size(buf_size) {};
    Slice(VA buf, size_t buf_size) : m_buf(reinterpret_cast<T *>(buf.get_raw())), m_size(buf_size) {};
    Slice(PA buf, size_t buf_size) : m_buf(reinterpret_cast<T *>(buf.get_raw())), m_size(buf_size) {};

    size_t size() { return m_size; }

    T& operator[](size_t idx) { return m_buf[idx]; }

    const T& operator[](size_t idx) const { return m_buf[idx]; }

    bool is_empty() { m_size == 0 || m_buf == nullptr; }

    T* get_buf() { return m_buf; }

    Slice sub_slice(size_t start, size_t end) {
        return Slice<T>(m_buf + start, end - start);
    }

    const T* begin() const { return m_buf; }
    const T* end() const { return m_buf + m_size; }
    T* begin() { return m_buf; }
    T* end() { return m_buf + m_size; }

    void fill(T c) {
        for (size_t i = 0; i < m_size; i++)
            m_buf[i] = c;
    }
};
