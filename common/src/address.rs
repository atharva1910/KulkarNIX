use core::ops::{Add, AddAssign, Sub, SubAssign};
use core::fmt::{self, LowerHex};

#[repr(transparent)]
#[derive(Clone, Default, Copy)]
pub struct VirtualAddress(pub u64);

impl VirtualAddress{
    pub fn pt_idx(&self) -> usize{
        (self.0 as usize >> 12) & 0x1FF
    }
    pub fn pdt_idx(&self) -> usize{
        (self.0 as usize >> 21) & 0x1FF
    }
    pub fn pdpt_idx(&self) -> usize{
        (self.0 as usize >> 30) & 0x1FF
    }
    pub fn pml4_idx(&self) -> usize{
        (self.0 as usize >> 39) & 0x1FF
    }
    pub fn get_raw(&self) -> u64 {
        self.0
    }
}

#[repr(transparent)]
#[derive(Clone, Default, Copy)]
pub struct PhysicalAddress(pub u64);

impl PhysicalAddress {
    pub fn get_raw(&self) -> u64 {
        self.0
    }
}

impl Add<u64> for VirtualAddress {
    type Output = Self;
    fn add(self, x: u64) -> Self::Output {
        Self(self.0 + x)
    }
}

impl Add<u64> for PhysicalAddress {
    type Output = Self;
    fn add(self, x: u64) -> Self::Output {
        Self(self.0 + x)
    }
}

impl AddAssign<u64> for VirtualAddress {
    fn add_assign(&mut self, rhs: u64) {
        self.0 += rhs;
    }
}

impl AddAssign<u64> for PhysicalAddress {
    fn add_assign(&mut self, rhs: u64) {
        self.0 += rhs;
    }
}

impl Sub<u64> for VirtualAddress {
    type Output = Self;
    fn sub(self, x: u64) -> Self::Output {
        Self(self.0 - x)
    }
}

impl Sub<u64> for PhysicalAddress {
    type Output = Self;
    fn sub(self, x: u64) -> Self::Output {
        Self(self.0 - x)
    }
}

impl SubAssign<u64> for VirtualAddress {
    fn sub_assign(&mut self, rhs: u64) {
        self.0 -= rhs;
    }
}

impl SubAssign<u64> for PhysicalAddress {
    fn sub_assign(&mut self, rhs: u64) {
        self.0 -= rhs;
    }
}

impl LowerHex for PhysicalAddress {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let _ = write!(f, "{:x}", self.0);
        Ok(())
    }
}

impl LowerHex for VirtualAddress {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let _ = write!(f, "{:x}", self.0);
        Ok(())
    }

}
