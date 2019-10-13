#include "HAL/HAL.h"
extern "C"{
#include "Interrupts.h"
}

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
  IDT[num].selector = 0x08;
  IDT[num].zero  = 0;
  IDT[num].offset_1 = (function & 0xffff);
  IDT[num].offset_2 = ((function >> 16) & 0xffff);
  IDT[num].type_attr = 0x8e; // TODOTODOTODO this is temp
}

 /*
SetupInterrupts
Description: This function sets up the IDT and Enables the Interrupts
This function sets up 256 entries of IDT fills them with defined interrupts
and fills the rest with Default IDT entry.
Then loads the entry

Arguments: 
  None
*/
void PIC::SetupInterrupts()
{
  InitIDT();
  InitDefaultIDT(); // for testing
  FillIDT();
  LoadIDT(reinterpret_cast<BYTE *>(&_idtr));
}

/*
Remap8059
Description: This function remaps the interrupts of the 8059 PIC away from the processor interrupts
Arguments : None
Return    : None
*/
void PIC::Remap8259()
{
}

PIC::PIC() {}
PIC::~PIC() {}

