#pragma once
#include <typedefs.h>

void HAL_outb(uint16_t port, uint8_t command);
uint8_t HAL_inb(uint16_t port);
void HAL_insw(uint16_t port, byte *address, uint32_t count);
void HAL_EnableInterrupts();
void HAL_DisableInterrupts();
void HAL_LoadIDT(byte *idtr);
void EnablePaging(uint32_t PDT);
bool CheckIfApicExists();
