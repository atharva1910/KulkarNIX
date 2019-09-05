#include "Debug.h"

/*
  This file conatins the code for all the interrupts in the IDT
  The asm functions jump to these Cpp functions to perform the interrupt
*/

extern "C"
void DefaultFunction()
{
    print_string("This is a default interrupt");
    while(true);
}

extern "C"
void KInterrupt000()
{
    print_string("This is a KInterrupt000");
    while(true);
}

extern "C"
void KInterrupt001()
{
    print_string("This is a KInterrupt001");
    while(true);
}

extern "C"
void KInterrupt002()
{
    print_string("This is a KInterrupt002");
    while(true);
}

extern "C"
void KInterrupt003()
{
    print_string("This is a KInterrupt003");
    while(true);
}

extern "C"
void KInterrupt004()
{
    print_string("This is a KInterrupt004");
    while(true);
}

extern "C"
void KInterrupt005()
{
    print_string("This is a KInterrupt005");
    while(true);
}

extern "C"
void KInterrupt006()
{
    print_string("This is a KInterrupt006");
    while(true);
}

extern "C"
void KInterrupt007()
{
    print_string("This is a KInterrupt007");
    while(true);
}

extern "C"
void KInterrupt008()
{
    print_string("This is a KInterrupt008");
    while(true);
}


