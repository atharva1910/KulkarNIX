#include "KPIC.h"

#define PIC1      0x20
#define PIC1_CMD  PIC1
#define PIC1_DATA (PIC1 + 1)

#define PIC2      0xA0
#define PIC2_CMD  PIC2
#define PIC2_DATA (PIC2 + 1)

void InitPIC()
{
    /*
     * ICW1: ((1 << 4) | 1)
     * IC4  : 1 : ICW4 is needed
     * SNGL : 0 : Cascade mode
     * ADI  : 0 : Vector address interval is 8
     * LTIM : 0 : Level Triggered
     * RESV : 1 : Reserved
     * A5 - A7 : 0 : Not applicable
     */
    HAL_outb(PIC1_CMD, ((1 << 4) | 1));
    HAL_outb(PIC2_CMD, ((1 << 4) | 1));

    /*
     * ICW2:
     * Since ADI in ICW1 is 0 i.e address interval is 8
     * A8 - A10 : 0 : Ignored
     * A11 - A15 : Offset into Interrupt Vector
     * Master will start at Interrupt vector 32 (0x20)
     * Slave will start at Interrupt vector 40 (0x28)
     */
    HAL_outb(PIC1_DATA, 0x20);
    HAL_outb(PIC2_DATA, 0x28);

    /*
     * ICW3:
     * Master :
     * We will setup so that Slave is on IRQ2
     * S0 - S7 : Bits indicate which IR has a slave
     *
     * Slave  :
     * Will have the ID 0x2
     */
    HAL_outb(PIC1_DATA, (1 << 2));
    HAL_outb(PIC2_DATA, (1 << 1));

    /*
     * ICW4:
     * Mode : 1 : 8086 Mode
     * D1 - D7 : 0 : Ignored (There is some meanings to this but we
     *                        ignore it for now)
     */
    HAL_outb(PIC1_DATA, 1);
    HAL_outb(PIC2_DATA, 1);

}

void
DisablePIC()
{
    HAL_outb(PIC1_DATA, 0xff);
    HAL_outb(PIC2_DATA, 0xff);
}

void
MaskIntr(uint32_t intrLine)
{

}

bool DoSomethingWithPic()
{
    // Maybe we want to protect with a lock?
    return FALSE;
}
