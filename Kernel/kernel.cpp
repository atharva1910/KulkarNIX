#include "typedefs.h"
#include "Debug.h"

extern "C"
void kernel_main()
{
    char const *c = "Welcome to the kernel";
    print_string(c);
    dump_address(0xBEEF);
    while(1);
}
