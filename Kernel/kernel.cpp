#include "typedefs.h"
#include "HAL/HAL.h"

void
InitInterrupts()
{
    HAL::SetupInterrupts();
    HAL::EnableInterrupts();
}

extern "C"
void kernel_main(void *memory_map)
{
    InitInterrupts();
    while(1);
}
