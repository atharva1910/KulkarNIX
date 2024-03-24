#pragma once
#include "typedefs.h"

struct _PIC;
typedef struct _PIC PIC, *PPIC;

PPIC AllocPIC();
void FreePIC();
