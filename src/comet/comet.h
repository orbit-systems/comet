#ifndef COMET_H
#define COMET_H

#include <stdio.h>

#define DPRINTF_EN 1

/* TODO: augment with thread ids */
#if DPRINTF_EN
#define DPRINTF(msg, ...) printf("%s %s:%d | DEBUG | " msg, __func__, __FILE__,__LINE__ __VA_OPT__(,) __VA_ARGS__)
#else
#define DPRINTF(...)
#endif

#define WPRINTF(msg, ...) printf("%s %s:%d | WARN | " msg, __func__, __FILE__,__LINE__ __VA_OPT__(,) __VA_ARGS__)

#endif // COMET_H