use r_efi::efi::{self, Guid, LOADER_DATA};
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

    pub fn halt(&self) -> efi::Status {
        unsafe {
            (self.get_bs().unwrap().stall)(usize::MAX)
        }
    }

    pub fn locate_protocol<T>(&self, mut guid: Guid) -> Option<*mut T> {
        let mut p = core::ptr::null_mut();
        let bs = BOOT_CTX.get_bs().unwrap();
        let  status = unsafe {
            (bs.locate_protocol)(&mut guid, core::ptr::null_mut(), &mut p)
        };

        if status != efi::Status::SUCCESS {
            return None;
        }

        Some(p as *mut T)
    }

    pub fn handle_protocol<T>(&self, h: efi::Handle, mut guid: Guid) -> Option<*mut T> {
        let mut p = core::ptr::null_mut();
        // TODO FIX THIS
        let bs = BOOT_CTX.get_bs().unwrap();
        let  status = unsafe {
            (bs.handle_protocol)(h, &mut guid, &mut p)
        };
        if status != efi::Status::SUCCESS {
            return None;
        }

        Some(p as *mut T)
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

    unsafe fn dealloc(&self, ptr: *mut u8, _: Layout) {
        if let Some(bs) =  BOOT_CTX.get_bs()  {
            unsafe {
                (bs.free_pool)(ptr.cast());
            }
        }
    }
}
