#include "Debug.h"

void
print_char(char *address, char c, BYTE bg_color)
{
    address[1] = bg_color;
    address[0] = c;
}

void
print_string(const char *string)
{
    char c = 0;
    uint32_t pos = 0;
    char *vga_buffer = (char *)0xb8000;
    while((c = string[pos++]) != '\0'){
        print_char(vga_buffer, c, 0x07);
        vga_buffer += 2;
    }
}

void
dump_address(uintptr_t address)
{
    char hex_addr[12] = "0x";
    uint32_t val = reinterpret_cast<uint32_t>(address);
    for(int i = 10; i >= 2; i--){
        uint8_t lower = val & 0xF;
        if (lower <= 9)  lower += 48;
        else             lower += 55; 
        hex_addr[i] = lower;
        val         = val >> 4;
    }
    print_string(hex_addr);
}


void
print_hex(char *addr)
{
    char hex_val[5] = "0x";

    char c = (addr[0] >> 4) & 0x0f;
    if (c >= 0 || c <= 9)
      c += 48 ;
    else
      c += 55; 
    hex_val[2] = c;

    c = addr[0] & 0x0f;
    if (c >= 0 || c <= 9)
      c += 48;
    else
      c += 55; 
    hex_val[3] = c;
    print_string(hex_val);
}


void
clrscr()
{
    uint32_t x = 80, y = 25;
    BYTE *vga_buffer = (BYTE *)0xb8000;
    char c = ' '; BYTE bg = 0x07;
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
