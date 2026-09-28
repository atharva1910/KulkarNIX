use core::{cell::UnsafeCell, ops::{Deref, DerefMut}, sync::atomic::{AtomicBool, Ordering}};

pub struct SpinLock<T> {
    data: UnsafeCell<T>,
    locked: AtomicBool,
}

pub struct SpinLockGuard<'a, T> {
    sp: &'a SpinLock<T>
}

unsafe impl<T: Send> Sync for SpinLock<T> {}

impl<T> SpinLock<T> {
    pub const fn init(data: T) -> Self {
        Self {
            data: UnsafeCell::new(data),
            locked: AtomicBool::new(false),
        }
    }

    pub fn lock(&self) -> SpinLockGuard<'_, T> {
        while self.locked.compare_exchange(false, true, Ordering::Acquire, Ordering::Relaxed).is_err() {
            core::hint::spin_loop();
        }

        SpinLockGuard {
            sp: self
        }
    }

    fn unlock(&self) {
        self.locked.store(false, Ordering::Release);
    }
}


impl<'a, T> Drop for SpinLockGuard<'a, T> {
    fn drop(&mut self) {
        self.sp.unlock();
    }
}

impl<'a, T> DerefMut for SpinLockGuard<'a, T> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        unsafe {
            &mut *self.sp.data.get()
        }
    }
}

impl<'a, T> Deref for SpinLockGuard<'a, T> {
    type Target = T;
    fn deref(&self) -> &Self::Target {
        unsafe {
            & *self.sp.data.get()
        }
    }
}
