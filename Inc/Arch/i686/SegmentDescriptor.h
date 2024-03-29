#pragma once

struct SegmentDescriptor {
    uint16_t limit;
    uint16_t base;
    uint8_t  base1;
    uint8_t  a       :1;
    uint8_t  rw      :1;
    uint8_t  ce      :1;
    uint8_t  type    :1;
    uint8_t  resrved :1;
    uint8_t  privl   :2;
    uint8_t  present :1;
    uint8_t  limit1  :4;
    uint8_t  avl     :1;
    uint8_t  lng     :1;
    uint8_t  big     :1;
    uint8_t  grn     :1;
    uint8_t  base2;
} __attribute__ ((packed));

struct GDT {
    SegmentDescriptor segment[3];
} __attribute__ ((packed));

struct GDTDescriptor {
    uint16_t sizeOfGDT;
    uint64_t gdtPtr;
} __attribute__((packed));
