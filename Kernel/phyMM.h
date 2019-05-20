#ifndef _PHYMM_H
#define _PHYMM_H
#include "typedefs.h"

#define SIZE_OF_BLOCK     4096    // Each block is of size 4096kb
#define BLOCK_ALLOCATED   0xFF
#define BLOCK_UNALLOCATED 0
#define MAX_SIZE          4294967296  // 4GB of address space
#define MAX_BLOCKS        (MAX_SIZE/SIZE_OF_BLOCK)
constexpr uint32_t bitmap_size = MAX_BLOCKS / 8;

static BYTE _memory_bit_map[bitmap_size];

// Global functions
// TODO: Should this return a MDL? Maybe or rename this to mm_alloc_page
void* mm_alloc_memory(size_t size);
void mm_free_memory();

// Local functions
static void mm_init_memory();
static void mm_set_memory();

#endif
