#include "typedefs.h"
#include "HAL/HAL.h"
#include "Debug/Debug.h"

void
InitInterrupts()
{
    PIC pic;
    //pic.Remap8259();
    pic.SetupInterrupts();
    HAL::EnableInterrupts();
}

void
SetupPaging()
{
    print_string("Setting up paging for x86");
    SetupX86Paging();
}

extern "C"
void kernel_main(void *memory_map)
{
    HAL::DisableInterrupts();
    SetupPaging();
    InitInterrupts();
    uintptr_t abc = reinterpret_cast<uintptr_t>(0xFFFFFFF);
    *abc = 9;
    // if(HAL::CheckIfApicExists())
    //     print_string("Exists");
    // else
    //     print_string("Nope");
    while(1);
}
