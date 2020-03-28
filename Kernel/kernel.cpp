#include "typedefs.h"
#include "HAL/HAL.h"
#include "Debug/Debug.h"

void
InitInterrupts()
{
    PIC pic;
    pic.Remap8259();
    pic.SetupInterrupts();
    HAL::EnableInterrupts();
}

extern "C"
void kernel_main(void *memory_map)
{
    //    InitInterrupts();
    if(HAL::CheckIfApicExists())
        print_string("Exists");
    else
        print_string("Nope");
    while(1);
}
