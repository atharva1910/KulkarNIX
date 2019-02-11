#include "Harddisk.h"
extern "C" void PPrintString();


void
ClearScreen()
{
    
}

bool
Harddisk::ReadWriteHarddisk(uint32_t sectorNum, uint8_t sectorCount, void *address, size_t bufferSize, bool write)
{
    if (sectorCount == 0 || bufferSize == 0)
        return false;

    bool bRet = false;
    uint32_t addressOffset = 0;
    for(uint8_t i = 0; i < sectorCount; i++){
        // Read each sector at address
        if(!ReadSector(sectorNum++, *(uint32_t *)address))
            return false;

        addressOffset += 512; // Point to the next address
        address += *(int *)addressOffset;
    }

    bRet = true;
    return bRet;
}

bool
Harddisk::ReadSector(uint32_t startSector, uint32_t address)
{
    // Send null to input and ouput port
    m_Port.outb(0x1F1, 0);
    // Set up sector count
    m_Port.outb(0x1F2, 1);
    // Set up LBAs
    m_Port.outb(0x1F3, (unsigned char) address);
    m_Port.outb(0x1F4, (unsigned char) address >> 8);
    m_Port.outb(0x1F5, (unsigned char) address >> 16);
    // Read Sector!
    m_Port.outb(0x1F7, 0x20);
    // wait for the disk
    while(!(m_Port.inb(0x1F7) &0xF7));
    m_Port.insw(0x1F0, 0x1000, 128);
    while(1);
    return true;
}

bool
Harddisk::WriteSector()
{
    return true;
}
