#include "boot_main.h"
#include "Harddisk.h"

extern "C" void
boot_main()
{
    Harddisk hdd;
    char buffer[10];
    hdd.ReadWriteHarddisk(0, 1, (void *)buffer, sizeof(buffer), true);
    while(1);
}
