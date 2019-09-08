#include "typedefs.h"
#include "HAL/HAL.h"

void
InitInterrupts()
{
    HAL::SetupAndEnableInterrupts();
}

extern "C"
void kernel_main(void *memory_map)
{
    InitInterrupts();
    while(1);
}
