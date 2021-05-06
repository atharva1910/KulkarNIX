#ifndef __BOOT_MAIN_H
#define __BOOT_MAIN_H
#define SECTOR_SIZE 512
#define KERNEL_START_SECT 5
#define KERNEL_START_PADDR 0x600000
#define KNIX_START_PAGE_ADDR 0x100000
#define KNIX_END_PAGE_ADDR 0x600000

#include "typedefs.h"
#include "elfheader.h"
#include "HAL/x86.h"

typedef struct _ACC_BYTE{
    uint8_t ac    : 1;
    uint8_t rw    : 1;
    uint8_t dc    : 1;
    uint8_t ex    : 1;
    uint8_t s     : 1;
    uint8_t privl : 1;
    uint8_t pr    : 1;
}AccByte, *PAccByte;

typedef struct _GDT{
    uint16_t limit    :15;
    uint16_t base     :15;
    uint8_t  base1;
    AccByte  accByte;
    uint8_t  limit1   :4;
    struct {
        uint8_t zero  :2;
        uint8_t sz    :1;
        uint8_t gr    :1;
    } __attribute__((packed)) flags;
    uint8_t  base2;
    
}GDT, *PGDT;
#endif
