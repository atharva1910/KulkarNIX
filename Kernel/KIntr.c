#include "KIntr.h"

typedef struct _IDTR {
    uint16_t IDTRSize;
    uint32_t pIDTR;
}__attribute__((packed)) IDTR;
IDTR idtr;

typedef struct _INTR_GATE {
    uint16_t offset1;
    uint16_t selector;
    union {
        uint8_t resv1;
        uint8_t resv2:5;
        uint8_t dpl:1;
        uint8_t p:1;
    }u;
    uint16_t offset2;
}INTR_GATE, *PINTR_GATE;
INTR_GATE InterruptTable[256];

void
AddInterrupts(uint8_t idx, uint32_t address)
{
    PINTR_GATE pGate = &InterruptTable[idx];
    pGate->offset1   = address & 0xFF;
    pGate->offset2   = (address >> 16) & 0xFF;
    pGate->u.resv1   = 0;
    pGate->u.resv2   = 0xe;
    pGate->u.p       = 1;
    pGate->u.dpl     = 1;
    pGate->selector  = 0;
}

void
InitInterrupts()
{
    idtr.pIDTR = (uint32_t)&InterruptTable;
    idtr.IDTRSize = sizeof(InterruptTable);

    /* Setup Exceptions and interrupts */

    /* Load IDTR */
    HAL_LoadIDT(&idtr);
}

void
DisableInterrupts()
{
    HAL_EnableInterrupts();
}

void
EnableInterrupts()
{
    HAL_EnableInterrupts();
}
