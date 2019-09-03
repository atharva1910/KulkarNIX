#ifndef __PAGING__H
#define __PAGING__H
#include "typedefs.h"

uint32_t page_dir[1024];

void paging_init();
void memory_init();

#endif
