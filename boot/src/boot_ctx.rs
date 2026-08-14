use r_efi::efi::{self, LOADER_DATA};
use core::alloc::{GlobalAlloc, Layout};
use core::sync::atomic::{AtomicPtr, Ordering};

pub struct BootCtx {
    st: AtomicPtr<efi::SystemTable>,
    rs: AtomicPtr<efi::RuntimeServices>,
    bs: AtomicPtr<efi::BootServices>,
}

#[global_allocator]
pub static BOOT_CTX: BootCtx = BootCtx {
    st: AtomicPtr::new(core::ptr::null_mut()),
    rs: AtomicPtr::new(core::ptr::null_mut()),
    bs: AtomicPtr::new(core::ptr::null_mut()),
};

impl BootCtx {
    pub fn new(&self, st: *mut efi::SystemTable) {
        BOOT_CTX.st.store(st, Ordering::Release);
        unsafe {
            BOOT_CTX.rs.store((*st).runtime_services, Ordering::Release);
            BOOT_CTX.bs.store((*st).boot_services, Ordering::Release);
        }
    }

    pub fn get_bs(&self) -> Option<&efi::BootServices> {
        unsafe {
            BOOT_CTX.bs.load(Ordering::Acquire).as_ref()
        }
    }

    pub fn get_st(&self) -> Option<&efi::SystemTable> {
        unsafe {
            BOOT_CTX.st.load(Ordering::Acquire).as_ref()
        }
    }
}

unsafe impl GlobalAlloc for BootCtx {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        let Some(bs) =  BOOT_CTX.get_bs() else {
            return core::ptr::null_mut();
        };

        let mut p = core::ptr::null_mut();
        if unsafe {
            (bs.allocate_pool)(LOADER_DATA, layout.size(), &mut p)
        }!= efi::Status::SUCCESS {
            return core::ptr::null_mut();
        }

        p.cast()
    }

    unsafe fn dealloc(&self, ptr: *mut u8, layout: Layout) {
        if let Some(bs) =  BOOT_CTX.get_bs()  {
            unsafe {
                (bs.free_pool)(ptr.cast());
            }
        }
    }
}
