#pragma once

#include <KStdArgs.h>

typedef enum _KPrintLevel {
    KVERBOSE,
    KINFO,
    KERR,
    KCRIT,
}KPrintLevel;

void
KPrint(KPrintLevel level, char *str, ...);
