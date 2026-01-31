#include "lock.h"
#include <errno.h>
#include <assert.h>
#include <pthread.h>

inline void comet_lock(pthread_mutex_t* lock) {
    int retval = pthread_mutex_lock(lock);
    switch (retval) {
    case EINVAL:
        assert("Attempted to lock a mutex that has not been initialised!");
    case EAGAIN:
        assert("Maximum number of recursive locks for mutex has been exceeded!");
    case EDEADLK:
        assert("Attempted to lock a mutex thats already locked by the locking thread!");
    default:
        return;
    }
}

inline void comet_unlock(pthread_mutex_t* lock) {
    int retval = pthread_mutex_unlock(lock);
    switch (retval) {
    case EINVAL:
        assert("Attempted to lock a mutex that has not been initialised!");
    case EAGAIN:
        assert("Maximum number of recursive locks for mutex has been exceeded!");
    case EPERM:
        assert("Thread attempted to unlock a mutex that it does not own!");
    default:
        return;
    }

}