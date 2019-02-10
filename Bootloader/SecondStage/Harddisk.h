#ifndef __HARDDISK_H
#define __HARDDISK_H
#include "Port.h"
#include "typedefs.h"

class Harddisk{
public:
    bool ReadWriteHarddisk(uint32_t address, uint8_t sectorCount, void *buffer, size_t bufferSize, bool write = false);
    Harddisk(){
    };
private:
    bool ReadSector();
    bool WriteSector();

    Port m_Port;
};
    
#endif
