#include <typedefs.h>
#include "KPrint.h"

/* Save allocating on stack every instance */
static char buffer[1024];

/* Allowed Log Level */
static KPrintLevel allowedLvl = KVERB;

/* VGA Buffer */
static char *vgaBuffer = (char *)0xB8000;

/* Screensize */
#define MAX_PAGES 8
#define MAX_ROWS  25
#define MAX_COLS  80
uint32_t col = 0;
uint32_t row = 0;
// TODO: When adding scrolling, for now we will keep writing to the same page
//#define MAX_ROWS  (25 * MAX_PAGES)

static inline void
PrintCharAt(char *address, char c, byte bg_color)
{
    address[1] = bg_color;
    address[0] = c;
}

static inline void
PrintChar(char c, byte bg_color)
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
            // TODO: Add pages and scrolling
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

int
reverse(int number, int radix)
{
    int ret = 0;
    while(number) {
        ret = ret * radix + (number % radix);
        number = number/radix;
    }
    return ret;
}

char *itoa_dec(int number, char *buffer)
{
    do {
        int tmp = number % 10;
        number  = number / 10;
        *buffer = tmp + '0';
        buffer++;
    } while(number);

    return buffer;
}

char *itoa(int number, char *buffer, int radix)
{
    number = reverse(number, radix);
    do {
        int tmp = number % radix;
        number  = number / radix;
        if (tmp < 10)
            *buffer = tmp + '0';
        else
            *buffer = tmp + 'a' - 10;
        buffer++;
    } while(number);

    return buffer;
}

char *
itoa1(int number, char *buffer, int radix)
{
    if (buffer == NULL)
        return NULL;

    if (radix == 10) {
        return itoa_dec(reverse(number, radix), buffer);
    } else if (radix == 16) {
        //return itoa_hex(reverse(number, radix), buffer);
    } else {
        return NULL;
    }
}

int
vsprintf(char *buf, char *str, va_list args)
{
    if (buf == NULL || str == NULL)
        return -1;

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

        /* We hit a % lets get the "type" of the argument to be printed */
        switch(*(++itr)) {
        case 'd': {
            head = itoa(va_arg(args, int), head, 10);
            if (head == NULL)
                return -1;
            itr++;
        } break;

        case 'x': {
            head = itoa(va_arg(args, int), head, 16);
            if (head == NULL)
                return -1;
            itr++;
        } break;

        default : {
            return -1;
        }
        }
    }

    *head = '\0';
    return 0;
}

void
KPrint(KPrintLevel level, char *str, ...)
{
    if (level < allowedLvl)
        return;

    va_list valist;

    va_start(valist, str);
    if (vsprintf(buffer, str, valist) == 0)
        PrintString(buffer);
    va_end(valist);
}
