pub const segment = packed struct {
    LimitLow: u16,
    BaseLow: u16,
    BaseMid: u8,
    Access: u8,
    LimitHigh: u4,
    Flags: u4,
    BaseLast: u8,
};
