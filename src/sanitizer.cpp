#include "sanitizer.h"

#include <iostream>

#include <dlfcn.h>
#include <pthread.h>
#include <sys/types.h> // pid_t

#include <stdio.h>

#include "id.h"
#include "orig_pthread.h"
#include "sentinel.h"

Sentinel sentinel(0);

pthread_mutex_init_type orig_pthread_mutex_init;
pthread_mutex_lock_type orig_pthread_mutex_lock;
pthread_mutex_unlock_type orig_pthread_mutex_unlock;
pthread_mutex_destroy_type orig_pthread_mutex_destroy;

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) noexcept {
    muid_t muid = (muid_t)mutex;
    sentinel.register_mutex(muid);

    return orig_pthread_mutex_init(mutex, mutexattr);
}

int pthread_mutex_lock(pthread_mutex_t *mutex) noexcept {
    pid_t tid = gettid();
    muid_t muid = (muid_t)mutex;

    auto result = sentinel.add_mutex_lock(tid, muid);
    if (result.has_value()) {
        auto err = result.value();
        std::cerr << err.format() << std::endl;
        exit(1);
    }

    return orig_pthread_mutex_lock(mutex);
}

int pthread_mutex_unlock(pthread_mutex_t *mutex) noexcept {
    pid_t tid = gettid();
    muid_t muid = (muid_t)mutex;
    sentinel.remove_mutex_lock(tid, muid);

    return orig_pthread_mutex_unlock(mutex);
}

int pthread_mutex_destroy(pthread_mutex_t *mutex) noexcept { return orig_pthread_mutex_destroy(mutex); }

void __attribute__((constructor)) sanitizer_init() {
    orig_pthread_mutex_init = (pthread_mutex_init_type)dlsym(RTLD_NEXT, "pthread_mutex_init");
    orig_pthread_mutex_lock = (pthread_mutex_lock_type)dlsym(RTLD_NEXT, "pthread_mutex_lock");
    orig_pthread_mutex_unlock = (pthread_mutex_lock_type)dlsym(RTLD_NEXT, "pthread_mutex_unlock");
    orig_pthread_mutex_destroy = (pthread_mutex_lock_type)dlsym(RTLD_NEXT, "pthread_mutex_destroy");
}
