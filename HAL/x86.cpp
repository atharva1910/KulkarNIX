#include "HAL/HAL.h"
//#include "x86lib.h"

namespace HAL {
void EnableInterrupts()
{
  asm volatile("sti");
}

void DisableInterrupts()
{
  asm volatile("cli");
}

} // namespace x86
