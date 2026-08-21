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
    pub const FIFO_CTRL_REG: Offset = Offset(3);
    pub const MODEM_CTRL_REG: Offset = Offset(4);
    pub const LINE_STATUS_REG: Offset = Offset(5);
    pub const MODEM_STATUS_REG: Offset = Offset(6);
    pub const SCRATCH_REG: Offset = Offset(7);
}

pub struct SerialPort;
pub static SERIAL_PORT: SerialPort = SerialPort;

impl SerialPort {
    pub fn init() {
        unsafe {
            hal::outb(COM1 + Offset::INT_ENABLE_REG.0, 0x0);
        }
    }
}
