#pragma once

typedef __gnuc_va_list va_list;

/* This just will not work on amd64 since arguments are not passed on stack */
#define va_start(ap, last) __builtin_va_start(ap, last)
#define va_arg(ap, type)   __builtin_va_arg(ap, type)
#define va_end(ap)         __builtin_va_end(ap)
