#ifndef _APIC_H
#define _APIC_H
#include "typedefs.h"
#include "PIO.h"
#define IDT_MAX_INTERRUPTS 256

class APIC : public PIO {
 public:
    APIC();
    ~APIC();
    void SetupInterrupts();
    void Remap8259();

 private:
    void InitIDT();
    void InitDefaultIDT();
    void FillIDT();
    void AddIDTEntry(uint8_t num, uintptr_t function);
    void LoadIDT(BYTE *idt);

 public:
    // Each entry in the IDT
    struct idt_entry{
      uint16_t offset_1;    // 0...15
      uint16_t selector;    // code segment 0x8 maybe?
      uint8_t  zero;        // 0
      uint8_t  type_attr;   // type and attr
      uint16_t offset_2;    // 16...31
    }__attribute__((packed));
    
    // The structure to be loaded using lidt
    struct idtr{
      uint16_t limit;       // Size of IDT
      uintptr_t base;        // Start address of IDT
    }__attribute__((packed));

 private:
    struct idtr      _idtr;
    static struct idt_entry IDT[IDT_MAX_INTERRUPTS];

    const uint32_t master_command = 0x20;
    const uint32_t salve_command  = 0xA0;
    const uint32_t master_data    = 0x21;
    const uint32_t salve_data     = 0xA1;

};

#endif
