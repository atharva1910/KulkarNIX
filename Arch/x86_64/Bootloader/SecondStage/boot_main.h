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

typedef struct _DataSegmentDesc{
    uint16_t limit    :15;
    uint16_t base     :15;
    uint8_t  base1;
    struct{
        uint8_t a       : 1;
        uint8_t rw      : 1;
        uint8_t ce      : 1;
        uint8_t type    : 1;
        uint8_t resrved : 1;
        uint8_t privl   : 2;
        uint8_t present : 1;
    }accByte;
    uint8_t  limit1     :4;
    struct {
        uint8_t avl     :1;
        uint8_t lng     :1;
        uint8_t big     :1;
        uint8_t grn     :1;
    }flags;
    uint8_t  base2;
    
}DataSegmentDesc, *PDataSegmentDesc;

typedef struct _CodeSegmentDesc{
    uint16_t limit    :15;
    uint16_t base     :15;
    uint8_t  base1;
    struct{
        uint8_t a       : 1;
        uint8_t rw      : 1;
        uint8_t ce      : 1;
        uint8_t type    : 1;
        uint8_t resrved : 1;
        uint8_t privl   : 2;
        uint8_t present : 1;
    }accByte;
    uint8_t  limit1     :4;
    struct {
        uint8_t avl     :1;
        uint8_t lng     :1;
        uint8_t big     :1;
        uint8_t grn     :1;
    }flags;
    uint8_t  base2;
    
}CodeSegmentDesc, *PCodeSegmentDesc;

typedef struct _GDT {
    CodeSegmentDesc nullSegment;
    CodeSegmentDesc codeSegment;
    DataSegmentDesc dataSegment;
}GDT, *PGDT;

typedef struct _GDTDescriptor {
    uint16_t sizeOfGDT;
    uint32_t gdtPtr;
}GDTDescriptor, *PGDTDescriptor;
#endif
