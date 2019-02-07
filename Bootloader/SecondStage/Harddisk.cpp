#include "Harddisk.h"

bool
Harddisk::ReadWriteHarddisk(uint32_t address, uint8_t sectorCount, void *buffer, size_t bufferSize, bool write)
{
    if (sectorCount == 0 || bufferSize == 0 || !bufferSize)
        return false;

    if(write)
        PPrintString();

    return ReadSector();
}

bool
Harddisk::ReadSector()
{
    return true;
}

bool
Harddisk::WriteSector()
{
    return true;
}
