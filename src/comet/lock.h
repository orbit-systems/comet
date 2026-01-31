#ifndef LOCK_H
#define LOCK_H

#include <pthread.h>

/// Attempt to lock a mutex
void comet_lock(pthread_mutex_t* lock);

/// Attempt to unlock a mutex
void comet_unlock(pthread_mutex_t* lock);

#endif