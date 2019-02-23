#ifndef __BOOT_MAIN_H
#define __BOOT_MAIN_H
    
#define DEBUG 1
#include "typedefs.h"
#include "x86.h"
#if DEBUG
#include "Debug.h"
#endif
#include "elfheader.h"

bool blank();

#endif
