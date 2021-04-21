#include "Debug/Debug.h"
/*
  This file conatins the code for all the interrupts in the IDT
  The asm functions jump to these Cpp functions to perform the interrupt
*/

// This macro creates an asm interrupt routine which calls the C++ code
#define ISR(NUM) \
    __asm__ (\
        ".global Interrupt"#NUM"\n"\
        "Interrupt"#NUM":\n"\
        "   call KInterrupt"#NUM"\n"\
        "   iret\n"\
             );    \

ISR(000)
extern "C"
void KInterrupt000()
{
}

ISR(001)
extern "C"
void KInterrupt001()
{
    asm volatile("hlt");
}

ISR(002)
extern "C"
void KInterrupt002()
{
}

ISR(003)
extern "C"
void KInterrupt003()
{
}

ISR(004)
extern "C"
void KInterrupt004()
{
}

ISR(005)
extern "C"
void KInterrupt005()
{
}

ISR(006)
extern "C"
void KInterrupt006()
{
}

ISR(007)
extern "C"
void KInterrupt007()
{
}

ISR(008)
extern "C"
void KInterrupt008()
{
}

__asm__(
        ".global DefaultISR\n"
        "DefaultISR:\n"
        "   call KDefault;\n"
        "   iret"
        );
extern "C"
void KDefault()
{
    asm volatile("hlt");
}

