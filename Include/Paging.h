#ifndef _KPAGE_H
#define _KPAGE_H
#include "typedefs.h"

static uint32_t *PML4, *PDPT, *PDT, PT;
void SetupX64Paging();

#endif
