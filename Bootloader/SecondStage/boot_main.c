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
    char buffer[255] = "Welcome to KulkarNIX";
    clrscr();
    print_string(buffer);
    read_kernel(address);
    while(1);
}
