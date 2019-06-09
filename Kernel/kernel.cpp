#include "typedefs.h"
#include "Debug.h"
extern "C" {
#include "HAL/IDT.h"
#include "HAL/x86.h"
}

void
InitInterrupts()
{
  LoadEmptyIDT();
  asm_enable_interrupts();
}

extern "C"
void kernel_main(void *memory_map)
{
  InitInterrupts();
  while(1);
}
