#include "KEntry.h"

/* Save allocating on stack every instance */
static char buffer[1024];

/* Position of the cursor */
static uint32_t bufPos = 0;

/* Allowed Log Level */
static KPrintLevel allowedLvl = KVERBOSE;

static inline void
PrintChar(char *address, char c, BYTE bg_color)
{
    address[1] = bg_color;
    address[0] = c;
}


static inline void
PrintString(const char *string)
{
    char *vga_buffer = (char *)0xb8000;
    char c = 0;
    uint32_t pos = 0;
    while((c = string[pos++]) != '\0'){
        PrintChar(vga_buffer, c, 0x07);
        vga_buffer += 2;
    }
}

void
vsprintf(char *buf, char *str, va_list args)
{
    if (buf == NULL || str == NULL)
        return;

    char *itr = str;
    char *head = buf;

    while (*itr != '\0') {
        /* keep copying until we hit a '%' */
        if (*itr != '%') {
            *head = *itr;
            head++;
            itr++;
            continue;
        }
    }
}

void
KPrint(KPrintLevel level, char *str, ...)
{
    if (level < allowedLvl)
        return;

    va_list valist;

    va_start(valist, str);
    vsprintf(buffer, str, valist);
    va_end(valist);

    PrintString(buffer);
}
