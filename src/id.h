#pragma once

#include <cstddef>
#include <sys/syscall.h>
#include <unistd.h>

typedef size_t muid_t;

#ifdef SYS_gettid
#define gettid() syscall(SYS_gettid)
#else
#error "SYS_gettid unavailable on this system"
#endif
