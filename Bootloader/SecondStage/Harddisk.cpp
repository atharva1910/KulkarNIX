#include "Harddisk.h"
bool
Harddisk::ReadWriteHarddisk(uint32_t address, uint8_t sectorCount, void *buffer, size_t bufferSize, bool write)
{
    return WriteSector();
    if (sectorCount == 0 || bufferSize == 0 || !bufferSize)
        return false;

}

bool
Harddisk::ReadSector()
{
    return true;
}

bool
Harddisk::WriteSector()
{
    char a = 'x';
    m_Port.outb(0x77, a);
    return true;
}

