#[repr(transparent)]
pub struct VirtualAddress(pub u64);

#[repr(transparent)]
pub struct PhysicalAddress(pub u64);

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
}
