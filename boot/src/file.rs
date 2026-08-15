use r_efi::{efi, protocols::{simple_file_system, file}};
pub struct EfiFile {
    file_handle: *mut file::Protocol,
    vol_handle: *mut file::Protocol,
}
use alloc::vec::Vec;

impl EfiFile {
    pub fn open_handle(sfs: *mut simple_file_system::Protocol, file_name: &str) -> Option<Self> {
        let mut vhandle: *mut file::Protocol = core::ptr::null_mut();
        let mut status = unsafe {((*sfs).open_volume)(sfs, &mut vhandle)};
        if status != efi::Status::SUCCESS {
            return None;
        }

        let mut fhandle: *mut file::Protocol = core::ptr::null_mut();
        let mut name: Vec<u16> = file_name.encode_utf16().collect();
        name.push(0);
        status = unsafe {((*vhandle).open)(vhandle, &mut fhandle, name.as_mut_ptr(), 1, 1)};
        if status != efi::Status::SUCCESS {
            return None;
        }

        Some(Self{
            file_handle: fhandle,
            vol_handle: vhandle,
        })
    }

    pub fn seek(&self, pos: usize) -> efi::Status {
        unsafe {
            ((*self.file_handle).set_position)(self.file_handle, pos as u64)
        }
    }

    pub fn read(&self, mut size: usize, buffer: *mut core::ffi::c_void) {
//pub type ProtocolRead = unsafe extern "efiapi" fn(
//    *mut Protocol,
//    *mut usize,
//    *mut core::ffi::c_void,
//) -> crate::base::Status;

        unsafe {
            ((*self.file_handle).read)(self.file_handle, &mut size, buffer);
        }
    }
}

impl Drop for EfiFile {
    fn drop (&mut self) {
        unsafe {
            (((*self.file_handle).close)(self.file_handle));
            (((*self.vol_handle).close)(self.vol_handle));
        }
    }
}
