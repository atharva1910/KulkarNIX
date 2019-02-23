#ifndef _DEBUG_H
#define _DEBUG_H
#include "typedefs.h"

void
print_char(char *address, char c, byte bg_color)
{
    address[1] = bg_color;
    address[0] = c;
}

void
print_string(char *string)
{
    byte bgcolor = 0x07;
    char *vga_buffer = (char *)0xb8000;
    char c = 0;
    uint32_t pos = 0;
    while((c = string[pos++]) != '\0'){
        print_char(vga_buffer, c, 0x07);
        vga_buffer += 2;
    }
}

void
clrscr()
{
    uint32_t x = 80, y = 25;
    byte *vga_buffer = (byte *)0xb8000;
    char c = ' '; byte bg = 0x07;
    uint32_t pos = 0;
    while(pos < x* y){
        vga_buffer[pos++] = c;
        vga_buffer[pos++] = bg;
    }
}

void
itoa(uint32_t number, char *buffer)
{
    if (buffer == NULL)
        return;

    uint32_t num = number;
    uint32_t digit = 0;
    uint32_t i = 0;
    while(num > 0){
        digit = num % 10;
        buffer[i++] = digit + '0';
        num = num / 10;
    }

    buffer[i] = '\0';
}

#endif
