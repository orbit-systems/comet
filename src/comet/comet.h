#ifndef COMET_H
#define COMET_H

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h> 

#include "common/portability.h"

/// Enable debug printing
#define DPRINTF_EN 1

/// Enable message passing debug printing
#define MDPRINTF_EN 1

/// Enable core debug printing
#define CDPRINTF_EN 1

/// Enable system debug printing
#define SDPRINTF_EN 1

/* NOTE: The following functions generate warnings when compiled. They can be ignored. */

static inline int comet_get_tid(void) {
#ifdef OS_LINUX
    // no glibc wrapper exists. augh.
    return syscall(SYS_gettid);
#elifdef OS_MACOS
    return pthread_getthreadid_np();
#else
#error Could not find suitable thread id function!
#endif
}

#if DPRINTF_EN
#define DPRINTF(msg, ...) printf("%s %s:%d | DEBUG %d | " msg, __func__, __FILE__,__LINE__, comet_get_tid() __VA_OPT__(,) __VA_ARGS__)
#else
#define DPRINTF(...)
#endif

#if MDPRINTF_EN
#define MDPRINTF(msg, ...) printf("%s %s:%d | MDEBUG %d | " msg, __func__, __FILE__,__LINE__, comet_get_tid() __VA_OPT__(,) __VA_ARGS__)
#else
#define MDPRINTF(...)
#endif

#if CDPRINTF_EN
#define CDPRINTF(msg, ...) printf("%s %s:%d | CDEBUG %d | " msg, __func__, __FILE__,__LINE__, comet_get_tid() __VA_OPT__(,) __VA_ARGS__)
#else
#define CDPRINTF(...)
#endif

#if SDPRINTF_EN
#define SDPRINTF(msg, ...) printf("%s %s:%d | SDEBUG %d | " msg, __func__, __FILE__,__LINE__, comet_get_tid() __VA_OPT__(,) __VA_ARGS__)
#else
#define SDPRINTF(...)
#endif


#define WPRINTF(msg, ...) printf("%s %s:%d | WARN %d | " msg, __func__, __FILE__,__LINE__, comet_get_tid() __VA_OPT__(,) __VA_ARGS__)

#endif // COMET_H