#pragma once

#include <pthread.h>

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) noexcept;
int pthread_mutex_lock(pthread_mutex_t *mutex) noexcept;
int pthread_mutex_unlock(pthread_mutex_t *mutex) noexcept;
int pthread_mutex_destroy(pthread_mutex_t *mutex) noexcept;
