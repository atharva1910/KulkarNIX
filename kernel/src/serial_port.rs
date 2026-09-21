use core::fmt::{self, Write};
use crate::hal;

const COM1: u16 = 0x3F8;
const BUFFER: u16 = COM1 + 0;
const INT_ENABLE_REG: u16 = COM1 + 1;
const DIVISOR_LSB: u16 = COM1 + 0;
const DIVISOR_MSB: u16 = COM1 + 1;
const INT_ID: u16 = COM1 + 2;
const FIFO_CTRL_REG: u16 = COM1 + 2;
const LINE_CTRL_REG: u16 = COM1 + 3;
const MODEM_CTRL_REG: u16 = COM1 + 4;
const LINE_STATUS_REG: u16 = COM1 + 5;
const MODEM_STATUS_REG: u16 = COM1 + 6;
const SCRATCH_REG: u16 = COM1 + 7;

struct Offset(pub u16);

pub struct SerialPort {}

pub static mut SERIAL_PORT: SerialPort = SerialPort{};

impl SerialPort {
    pub fn init() -> bool {
        // Disable Interrupts
        hal::outb(INT_ENABLE_REG, 0x0);
        hal::outb(LINE_CTRL_REG, 0x80);
        hal::outb(DIVISOR_LSB, 0x3);
        hal::outb(DIVISOR_MSB, 0x0);
        hal::outb(LINE_CTRL_REG, 0x3);
        hal::outb(FIFO_CTRL_REG, 0xC7);
        hal::outb(MODEM_CTRL_REG, 0x0B);
        true
    }

    pub fn write_u8(byte: u8) {
        while hal::inb(LINE_STATUS_REG) & 0x20 == 0 {}
        hal::outb(BUFFER, byte);
    }

    pub fn write(x: &str) {
        for byte in x.bytes() {
            Self::write_u8(byte);
        }
    }

    pub fn read() -> u8 {
        while hal::inb(LINE_STATUS_REG) & 0x1 == 0 {}
        hal::inb(BUFFER)
    }
}

impl fmt::Write for SerialPort {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        SerialPort::write(s);
        Ok(())
    }
}
