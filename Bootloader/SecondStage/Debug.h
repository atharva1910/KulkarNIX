#ifndef _DEBUG_H
#define _DEBUG_H
#include "typedefs.h"

void
print_char(char *address, char c, byte bg_color)
{
    address[1] = bg_color; //black bg
    address[0] = c;
}

void
print_string(char *string)
{
    byte bgcolor = 0x07;
    char *vga_buffer = (char *)0xb8000;
    uint32_t pos = 0;
    while(string[pos] != '\0'){
        print_char(vga_buffer, string[pos], 0x07);
        vga_buffer += 2;
        pos++;
    }
}

void
clrscr()
{
    uint32_t x=80, y=25;
    byte *vga_buffer = (byte *)0xb8000;
    char c = ' '; byte bg = 0x07;
    uint32_t pos = 0;
    while(pos < x* y){
        vga_buffer[pos++] = c;
        vga_buffer[pos++] = bg;
    }
}
#endif
