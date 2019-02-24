#ifndef __BOOT_MAIN_H
#define __BOOT_MAIN_H
#define DEBUG 0
#define SECTOR_SIZE 512
#define KERNEL_START_SECT 5

#include "typedefs.h"
#include "x86.h"
#include "elfheader.h"

#if DEBUG
#include "Debug.h"
#endif

bool blank();

#endif
