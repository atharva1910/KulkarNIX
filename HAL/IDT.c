#include "HAL/IDT.h"
#include "IDTlib.h"

//////////////////////////////////////////////////////
//                Global functions                  //
//////////////////////////////////////////////////////

extern void asm_load_idt();

void LoadEmptyIDT()
{
  // Init the empty idt
  init_idt();
  // Load the empty idt
  load_idt();
}



//////////////////////////////////////////////////////
//                Local functions                   //
//////////////////////////////////////////////////////

struct idtr      _idtr;
static struct idt_entry IDT[IDT_MAX_INTERRUPTS];

static void init_idt()
{
  _idtr.limit = sizeof(struct idt_entry) * IDT_MAX_INTERRUPTS - 1; //This can really be a constexpr
  _idtr.base  = (uintptr_t)&IDT[0];

  // for(int i = 0; i < IDT_MAX_INTERRUPTS; i++) IDT[i] = {0}; // Kinda unecessary since satic global
}

static void load_idt()
{
  asm_load_idt();
}
