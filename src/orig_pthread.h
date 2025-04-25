// Globally accessible original pthread functions.
#pragma once

#include <pthread.h>

typedef int (*pthread_mutex_init_type)(pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);
extern pthread_mutex_init_type orig_pthread_mutex_init;

typedef int (*pthread_mutex_lock_type)(pthread_mutex_t *mutex);
extern pthread_mutex_lock_type orig_pthread_mutex_lock;

typedef int (*pthread_mutex_unlock_type)(pthread_mutex_t *mutex);
extern pthread_mutex_unlock_type orig_pthread_mutex_unlock;

typedef int (*pthread_mutex_destroy_type)(pthread_mutex_t *mutex);
extern pthread_mutex_destroy_type orig_pthread_mutex_destroy;
