#include "phyMM.h"
#include "KulkarNIX.h"

//////////////////////////////////////////////////////
//                GLOBAL FUNCTIONS                  //
//////////////////////////////////////////////////////
void* mm_alloc_memory(size_t size)
{
  UNREFRENCED_PARAMETER(size);
  void *ret_ptr = NULL;
  return ret_ptr;
  // Find the first unset bit in the array
  // Calculate the address of the bit
  // return the bit
}

void mm_free_memory()
{
  // Calculate the bit where this memory lies
  // Clear out the bit
}

//////////////////////////////////////////////////////
//                LOCAL  FUNCTIONS                  //
//////////////////////////////////////////////////////

/*
  This function will clear out all of the physical memory bitmaps to 0
*/
static void mm_init_memory()
{
  for (uint32_t idx = 0; idx < bitmap_size; idx++){
    _memory_bit_map[idx] = BLOCK_UNALLOCATED;
  }
}
