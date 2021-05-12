#ifndef __BOOT_MAIN_H
#define __BOOT_MAIN_H
#define SECTOR_SIZE 512
#define KERNEL_START_SECT 5
#define KERNEL_START_PADDR 0x600000
#define KNIX_START_PAGE_ADDR 0x100000
#define KNIX_END_PAGE_ADDR 0x600000
#include "typedefs.h"
#include "elfheader.h"
#include "x86.h"
#endif
