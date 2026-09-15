#pragma once

template <typename T> class Slice {
private:
  size_t m_size{0};
  T *m_buf{nullptr};

public:
  Slice(T *buf, size_t buf_size) : m_buf(buf), m_size(buf_size) {};

  size_t size() { return m_size; }

  T &operator[](size_t idx) { return m_buf[idx]; }

  const T &operator[](size_t idx) const { return m_buf[idx]; }

  bool is_empty() { m_size == 0 || m_buf == nullptr; }

  Slice sub_slice(size_t start, size_t end) {
      return Slice<T>(m_buf + start, end - start);
  }
};
