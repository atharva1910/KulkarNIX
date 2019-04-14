#include "typedefs.h"
#include "Debug.h"

extern "C"
void kernel_main(void *memory_map)
{
    char const *c = "Welcome to the kernel";
    print_string(c);
    dump_address((uintptr_t)memory_map);
    while(1);
}
