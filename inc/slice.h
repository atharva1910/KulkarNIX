#pragma once

template <typename T> class Slice {
private:
  size_t m_size{0};
  T *m_buf{nullptr};

public:
  Slice(T *buf, size_t buf_size) : m_buf(buf), m_size(buf_size) {};
  size_t size() { return m_size; }
  T &operator[](size_t idx) { return m_buf[idx]; }

  Slice sub_slice(size_t start, size_t end) {
      return Slice<T>(m_buf[start], end - start);
  }
};
