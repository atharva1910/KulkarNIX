#include "boot_main.h"

void read_sector(void *address)
{
}

void read_kernel(void *address)
{
}

void
boot_main()
{
    byte *address = (byte *)0x10000; // Save kernel at address
    read_kernel(address);
    while(1);
}
