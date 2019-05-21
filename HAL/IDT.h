#ifndef _IDT_H
#define _IDT_H
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
  uint32_t base;        // Start address of IDT
}__attribute__((packed));

idt_entry IDT[256];

static void init_idt();
static void load_idt();



// Global
void LoadIDT();
#endif
