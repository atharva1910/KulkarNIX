#pragma once

#include <stdarg.h>

typedef enum _KPrintLevel {
    KVERB,
    KINFO,
    KERR,
    KCRIT,
}KPrintLevel;

void
KPrint(KPrintLevel level, char *str, ...);
