#pragma once
#include "typedefs.h"

void outb(uint16_t port, uint8_t command);
uint8_t inb(uint16_t port);
void insw(uint16_t port, BYTE *address, uint32_t count);
void EnableInterrupts();
void DisableInterrupts();
void EnablePaging(uint32_t PDT);
BOOL CheckIfApicExists();
