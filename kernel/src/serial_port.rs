use core::fmt::{self, Write};
use crate::hal;

const COM1: u16 = 0x3F8;
const COM2: u16 = 0x2F8;

struct Offset(pub u16);
impl Offset {
    pub const BUFFER: Offset = Offset(0);
    pub const INT_ENABLE_REG: Offset = Offset(1);
    pub const DIVISOR_LSB: Offset = Offset(0);
    pub const DIVISOR_MSB: Offset = Offset(1);
    pub const INT_ID: Offset = Offset(2);
    pub const FIFO_CTRL_REG: Offset = Offset(2);
    pub const LINE_CTRL_REG: Offset = Offset(3);
    pub const MODEM_CTRL_REG: Offset = Offset(4);
    pub const LINE_STATUS_REG: Offset = Offset(5);
    pub const MODEM_STATUS_REG: Offset = Offset(6);
    pub const SCRATCH_REG: Offset = Offset(7);
}

pub struct SerialPort;
pub static mut SERIAL_PORT: SerialPort = SerialPort;

impl SerialPort {
    pub fn init() -> bool {
        // Disable Interrupts
        hal::outb(COM1 + Offset::INT_ENABLE_REG.0, 0x0);
        // Start Set BAUD Rate
        hal::outb(COM1 + Offset::LINE_CTRL_REG.0, 0x80);
        hal::outb(COM1 + Offset::DIVISOR_LSB.0, 0x3);
        hal::outb(COM1 + Offset::DIVISOR_MSB.0, 0x0);
        // 8 Bits, No parity, One stop bit
        hal::outb(COM1 + Offset::LINE_CTRL_REG.0, 0x3);

        // Enable FIFO, Clear Buffers
        hal::outb(COM1 + Offset::FIFO_CTRL_REG.0, 0xC7);
        // Int enable
        hal::outb(COM1 + Offset::MODEM_CTRL_REG.0, 0x0B);

        true
    }

    pub fn write(&self, x: &str) {
        for byte in x.bytes() {
            while hal::inb(COM1 + Offset::LINE_STATUS_REG.0) & 0x20 == 0 {}
            hal::outb(COM1 + Offset::BUFFER.0, byte);
        }
    }

    pub fn read() -> u8 {
        while hal::inb(COM1 + Offset::LINE_STATUS_REG.0) & 0x1 == 0 {}
        hal::inb(COM1 + Offset::BUFFER.0)
    }
}

impl fmt::Write for SerialPort {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        self.write(s);
        Ok(())
    }
}
