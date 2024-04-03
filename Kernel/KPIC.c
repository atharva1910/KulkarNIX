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

/*
PIC::idt_entry PIC::IDT[] = {0};

void PIC::LoadIDT(BYTE *idt)
{
    asm volatile("lidt %0"::"m"(*idt));
}


void PIC::InitIDT()
{
  _idtr.limit = sizeof(struct idt_entry) * IDT_MAX_INTERRUPTS - 1; //This can really be a constexpr
  _idtr.base  = (uintptr_t)&IDT[0];
}

void PIC::InitDefaultIDT()
{
  for (uint16_t i = 0; i < IDT_MAX_INTERRUPTS; i++){
    AddIDTEntry(i, (uintptr_t)DefaultISR);
  }
}

void PIC::FillIDT()
{
    AddIDTEntry(0, (uintptr_t)Interrupt000);
    AddIDTEntry(1, (uintptr_t)Interrupt001);
    AddIDTEntry(2, (uintptr_t)Interrupt002);
    AddIDTEntry(3, (uintptr_t)Interrupt003);
    AddIDTEntry(4, (uintptr_t)Interrupt004);
    AddIDTEntry(5, (uintptr_t)Interrupt005);
    AddIDTEntry(6, (uintptr_t)Interrupt006);
    AddIDTEntry(7, (uintptr_t)Interrupt007);
    AddIDTEntry(8, (uintptr_t)Interrupt008);
    //    AddIDTEntry(9, (uintptr_t)Interrupt09);
    //    AddIDTEntry(10, (uintptr_t)Interrupt10);
}

void PIC::AddIDTEntry(uint8_t num, uintptr_t function)
{
  uint64_t fun = reinterpret_cast<uint64_t>(function);
  IDT[num].selector = 0x08;
  IDT[num].zero  = 0;
  IDT[num].offset_1 = (fun & 0xffff);
  IDT[num].offset_2 = ((fun >> 16) & 0xffff);
  IDT[num].type_attr = 0x8e; // TODOTODOTODO this is temp
}


void PIC::SetupInterrupts()
{
  InitIDT();
  InitDefaultIDT(); // for testing
  FillIDT();
  LoadIDT(reinterpret_cast<BYTE *>(&_idtr));
}

Remap8059
Description: This function remaps the interrupts of the 8059 PIC away from the processor interrupts (0x00 - 0x1F)
Arguments : None
Return    : None

void PIC::Remap8259()
{

}

PIC::PIC() {}
PIC::~PIC() {}
*/
