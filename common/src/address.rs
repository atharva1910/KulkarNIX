use core::fmt::{self, LowerHex};
use core::ops::{Add, AddAssign, Sub, SubAssign};
use crate::KERNEL_DS_ADDR;

macro_rules! addr_functions {
    ($name:ident) => {
        #[repr(transparent)]
        #[derive(Clone, Default, Copy)]
        pub struct $name(pub usize);

        impl $name {
            pub fn get_raw(&self) -> usize {
                self.0
            }
            pub fn to_ptr<T>(&self) -> *const T {
                self.0 as *const T
            }
        }

        impl From<$name> for usize {
            fn from(addr: $name) -> usize {
                addr.0
            }
        }

        impl From<usize> for $name {
            fn from(addr: usize) -> $name {
                $name(addr)
            }
        }

        impl LowerHex for $name {
            fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
                let _ = write!(f, "{:x}", self.0);
                Ok(())
            }
        }

        impl Add<usize> for $name {
            type Output = $name;
            fn add(self, x: usize) -> Self::Output {
                $name(self.0 + x)
            }
        }

        impl Sub<usize> for $name {
            type Output = $name;
            fn sub(self, x: usize) -> Self::Output {
                $name(self.0 - x)
            }
        }


        impl AddAssign<usize> for $name {
            fn add_assign(&mut self, rhs: usize) {
                self.0 += rhs;
            }
        }

        impl SubAssign<usize> for $name {
            fn sub_assign(&mut self, x: usize) {
                self.0 -= x;
            }
        }
    };
}

impl VirtualAddress {
    pub fn pt_idx(&self) -> usize {
        (self.0 as usize >> 12) & 0x1FF
    }
    pub fn pdt_idx(&self) -> usize {
        (self.0 as usize >> 21) & 0x1FF
    }
    pub fn pdpt_idx(&self) -> usize {
        (self.0 as usize >> 30) & 0x1FF
    }
    pub fn pml4_idx(&self) -> usize {
        (self.0 as usize >> 39) & 0x1FF
    }
    pub fn to_physical(&self) -> PhysicalAddress {
        assert!(self.0 >= KERNEL_DS_ADDR);
        PhysicalAddress(self.0 - KERNEL_DS_ADDR)
    }
}

impl PhysicalAddress {
    pub fn to_virtual(&self) -> VirtualAddress {
        VirtualAddress(self.0 + KERNEL_DS_ADDR)
    }
}


impl From<PhysicalAddress> for VirtualAddress {
    fn from(addr: PhysicalAddress) -> VirtualAddress {
        VirtualAddress(addr.0 + KERNEL_DS_ADDR)
    }
}

impl From<VirtualAddress> for PhysicalAddress {
    fn from(addr: VirtualAddress) -> PhysicalAddress {
        PhysicalAddress(addr.0 - KERNEL_DS_ADDR)
    }
}



addr_functions!(VirtualAddress);
addr_functions!(PhysicalAddress);
