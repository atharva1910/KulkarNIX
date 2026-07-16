#pragma once

#include <stdarg.h>
#include <typedefs.h>

typedef enum _KPrintLevel {
    KVERB,
    KINFO,
    KERR,
    KCRIT,
}KPrintLevel;

void
KPrint(KPrintLevel level, char *str, ...);
