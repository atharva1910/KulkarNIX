#include "KEntry.h"

/* Save allocating on stack every instance */
static char buffer[1024];

/* Position of the cursor */
static uint32_t bufPos = 0;

/* Allowed Log Level */
static KPrintLevel allowedLvl = KVERBOSE;

/* VGA Buffer */
static char *vgaBuffer = (char *)0xB8000;

/* VGA Buffer Size (32KB)*/
static uint32_t vgaBufferSize = 32 * 1204;

/* Screensize */
#define MAX_PAGES 8
#define MAX_ROWS  25
#define MAX_COLS  80
uint32_t col = 0;
uint32_t row = 0;
// TODO: When adding scrolling, for now we will keep writing to the same page
//#define MAX_ROWS  (25 * MAX_PAGES)

static inline void
PrintCharAt(char *address, char c, BYTE bg_color)
{
    address[1] = bg_color;
    address[0] = c;
}

static inline void
PrintChar(char c, BYTE bg_color)
{
    uint32_t bufPos   = (row * MAX_COLS + col) * 2;
    char     *address = vgaBuffer + bufPos;

    address[1] = bg_color;
    address[0] = c;

    col++;
    if (col >= MAX_COLS) {
        col = 0;
        row++;
        if (row >= MAX_ROWS)
            row = 0;
    }
}

static void
PrintString(const char *string)
{
    char c = 0;
    uint32_t pos = 0;

    /* Print string to screen */
    while((c = string[pos++]) != '\0') {
        PrintChar(c, 0x07);
    }

    while(col != 0) PrintChar(0, 0x7);
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
