#ifndef _DEBUG_H
#define _DEBUG_H
#include "typedefs.h"

void print_char(char *address, char c, BYTE bg_color);
void print_string(const char *string);
void dump_address(uintptr_t address);
void print_hex(char *addr);
void clrscr();
void itoa(uint32_t number, char *buffer);
#endif
