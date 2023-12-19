#pragma once

typedef char *va_list;

#define va_size(type) (sizeof(type))
#define va_start(ap, last) ((ap) = (char *)&(last) + va_size(last))
#define va_arg(ap, type) (*(type *)(ap)++)
#define va_end(ap)
