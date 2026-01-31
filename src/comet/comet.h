#ifndef COMET_H
#define COMET_H

#include <stdio.h>

#define DPRINTF_EN 1

#if DPRINTF_EN
#define DPRINTF(msg, ...) printf("%s %s:%d | " msg, __func__, __FILE__,__LINE__ __VA_OPT__(,) __VA_ARGS__)
#endif

#endif // COMET_H