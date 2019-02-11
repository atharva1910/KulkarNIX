#include "boot_main.h"
#include "Harddisk.h"

extern "C" void
boot_main()
{
    Harddisk hdd;
    void *address = (void *)0x10000; // Save kernel at address
    size_t bufferSize = 512; // Sector size
    uint32_t sectorNum = 3;
    hdd.ReadWriteHarddisk(sectorNum, 1, (void *)address, bufferSize, false);
    while(1);
}
