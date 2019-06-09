#include "HAL/IDT.h"
#include "IDTlib.h"

//////////////////////////////////////////////////////
//                Global functions                  //
//////////////////////////////////////////////////////

extern void asm_load_idt();
extern void DefaultIDTfun();

void LoadEmptyIDT()
{
  // Init the empty idt
  InitIDT();

  // Load defualt IDT
  InitDefaultIDT();

  // Load the empty idt
  LoadIDT();
}



//////////////////////////////////////////////////////
//                Local functions                   //
//////////////////////////////////////////////////////

struct idtr      _idtr;
static struct idt_entry IDT[IDT_MAX_INTERRUPTS];

static void InitIDT()
{
  _idtr.limit = sizeof(struct idt_entry) * IDT_MAX_INTERRUPTS - 1; //This can really be a constexpr
  _idtr.base  = (uintptr_t)&IDT[0];

  // for(int i = 0; i < IDT_MAX_INTERRUPTS; i++) IDT[i] = {0}; // Kinda unecessary since satic global
}

static void LoadIDT()
{
  asm_load_idt();
}

static void InitDefaultIDT()
{
  for (uint16_t i = 0; i < IDT_MAX_INTERRUPTS; i++){
    AddIDTEntry(i, (uintptr_t)DefaultIDTfun);
  }
}

static void AddIDTEntry(uint8_t num, uintptr_t function)
{
  IDT[num].selector = 0x08;
  IDT[num].zero  = 0;
  IDT[num].offset_1 = (function & 0xffff);
  IDT[num].offset_2 = ((function >> 16) & 0xffff);
  IDT[num].type_attr = 0x8e; // TODOTODOTODO this is temp
}
