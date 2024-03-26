#pragma once
#include <x86.h>
#include "typedefs.h"

/*
InitPIC

Description:
  Performs intialization sequence for 8259A
  The master interrupts are mapped to (0x20 - 0x27)
  The slave interrupts are mapped to (0x28 - 0x2F)
  Intializes the master and slave 8259A PIC in cascade mode

Arguments:
  None
*/
void InitPIC();

/*
DisablePIC

Description:
  Disables the PIC. All interrupts are masked

Arguments:
  None
*/
void DisablePIC();


/*
MaskIntr

Description:
  Masks the interrupt line

Arguments:
  intrLine : Interrupt line to be mased
*/
void MaskIntr(uint32 intrLine);
