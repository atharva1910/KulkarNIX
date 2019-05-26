#ifndef _IDT_LIB_H
#define _IDT_LIB_H
#include "typedefs.h"

#define IDT_MAX_INTERRUPTS 256

// Each entry in the IDT
struct idt_entry{
  uint16_t offset_1;    // 0...15
  uint16_t selector;    // code segment 0x10 maybe?
  uint8_t  zero;        // 0
  uint8_t  type_attr;   // type and attr
  uint16_t offset_2;    // 16...31
}__attribute__((packed));

// The structure to be loaded using lidt
struct idtr{
  uint16_t limit;       // Size of IDT
  uintptr_t base;        // Start address of IDT
}__attribute__((packed));

static void init_idt();
static void load_idt();
#endif