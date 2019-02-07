#ifndef __HARDDISK_H
#define __HARDDISK_H

typedef unsigned int uint32_t;
typedef unsigned int size_t;
typedef unsigned char uint8_t;

extern "C" void PPrintString();
class Harddisk{
public:
    bool ReadWriteHarddisk(uint32_t address, uint8_t sectorCount, void *buffer, size_t bufferSize, bool write = false);
private:
    bool ReadSector();
    bool WriteSector();

    /* All the ports used by harddisk to communicate*/
};
    
#endif
