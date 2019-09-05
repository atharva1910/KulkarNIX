#include "Debug.h"

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
    print_string("This is a KInterrupt000");
    while(true);
}

ISR(001)
extern "C"
void KInterrupt001()
{
    print_string("This is a KInterrupt001");
    while(true);
}

ISR(002)
extern "C"
void KInterrupt002()
{
    print_string("This is a KInterrupt002");
    while(true);
}

ISR(003)
extern "C"
void KInterrupt003()
{
    print_string("This is a KInterrupt003");
    while(true);
}

ISR(004)
extern "C"
void KInterrupt004()
{
    print_string("This is a KInterrupt004");
    while(true);
}

ISR(005)
extern "C"
void KInterrupt005()
{
    print_string("This is a KInterrupt005");
    while(true);
}

ISR(006)
extern "C"
void KInterrupt006()
{
    print_string("This is a KInterrupt006");
    while(true);
}

ISR(007)
extern "C"
void KInterrupt007()
{
    print_string("This is a KInterrupt007");
    while(true);
}

ISR(008)
extern "C"
void KInterrupt008()
{
    print_string("This is a KInterrupt008");
    while(true);
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
    print_string("This is a default interrupt");
    while(true);
}

