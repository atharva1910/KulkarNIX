#include "KIntr.h"

struct _IDTR {
    uint16_t IDTRSize;
    uint32_t pIDTR;
}__attribute__((packed));

typedef struct _IDTR IDTR;

void
InitInterrupts()
{
}
