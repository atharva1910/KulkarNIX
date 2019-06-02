#include "typedefs.h"
#include "Debug.h"
extern "C" {
#include "HAL/IDT.h"
#include "HAL/x86.h"
}

void
SetupIDT()
{
  LoadEmptyIDT();
}

extern "C"
void kernel_main(void *memory_map)
{
  SetupIDT();
  while(1);
}
