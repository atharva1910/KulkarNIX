#include "typedefs.h"
#include "HAL/IDT.h"

void
InitInterrupts()
{
    PIC::SetupAndEnableInterrupts();
}

extern "C"
void kernel_main(void *memory_map)
{
    InitInterrupts();
    while(1);
}
