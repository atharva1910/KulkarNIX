#pragma once

class SpinLock {
  private:
    bool m_locked{false};

  public:
    void lock() {
        while(__atomic_test_and_set(&m_locked, __ATOMIC_ACQUIRE));
    }

    void unlock() {
        __atomic_clear(&m_locked, __ATOMIC_RELEASE);
    }
};


class LockGuard {
  private:
    SpinLock& m_lock;

  public:
    LockGuard(SpinLock& s) : m_lock(s) { m_lock.lock(); }
    ~LockGuard() { m_lock.unlock(); }
};
