#include "typedefs.h"
#include "Debug.h"
#include "HAL/IDT.h"
#include "HAL/x86.h"

void
InitInterrupts()
{
  SetupInterrupts();
  asm_enable_interrupts();
}

extern "C"
void kernel_main(void *memory_map)
{
    memory_map = memory_map;
  InitInterrupts();
  while(1);
}
