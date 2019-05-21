#include "IDT.h"

//////////////////////////////////////////////////////
//                Global functions                  //
//////////////////////////////////////////////////////

void LoadIDT()
{
  // Init the empty idt
  init_idt();
  // Load the empty idt
  load_idt();
}



//////////////////////////////////////////////////////
//                Local functions                   //
//////////////////////////////////////////////////////

static idtr      _idtr;
static idt_entry IDT[IDT_MAX_INTERRUPTS];

static void init_idt()
{
  _idtr.limit = sizeof(idt_entry) * IDT_MAX_INTERRUPTS - 1; //This can really be a constexpr
  _idtr.base  = &IDT[0];

  for(int i = 0; i < IDT_MAX_INTERRUPTS; i++) IDT[i] = 0; // Kinda unecessary since satic global
}

static void load_idt()
{
  asm_load_idt(&_idtr);
}
