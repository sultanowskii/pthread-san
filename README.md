# pthread-san

`pthread` sanitizer. Is based on the [Google TSan Deadlock Detector](https://github.com/google/sanitizers/wiki/ThreadSanitizerDeadlockDetector).

It detects deadlocks caused by improper mutex lock order.

No `#include`s or extra dependencies are required - all you have to do is `LD_PRELOAD` the sanitizer. See the usage example below.

Supports C/C++ (and might also languages like Rust, Zig, etc.) programs - all as long as they use `pthread` - either explicitly (`#include <pthread.h>`) or implicitly (e.g. `std::mutex` on my machine uses `pthread` under the hood).

## Requirements

`g++`

## Build

```bash
git clone https://github.com/sultanowskii/pthread-san
cd pthread-san
make
```

## Usage

```bash
LD_PRELOAD=./sanitizer.so ./your-program
```

## Example

See the [examples/](examples/) directory.

The code below is problematic - mu1 and mu2 could be acquired in a different order. This causes potential lock() "loop" and, in the end, might lead to deadlock.

Examples are compiled in a standard manner (`-rdynamic` is added for more verbose deadlock report):

```bash
$ make build-examples
g++ -rdynamic -o examples/cpp-cycle-example.elf examples/cpp-cycle-example.cpp
cc -rdynamic -o examples/c-cycle-example.elf examples/c-cycle-example.c
$
```

And then run it, specifying `sanitizer.so` in `LD_PRELOAD`:

```bash
$ LD_PRELOAD=./sanitizer.so examples/c-cycle-example.elf
hi from f1()!
Cycle in lock order graph: M1 [0x55e16f0400c0] => M0 [0x55e16f040080] => M1 [0x55e16f0400c0]
---=== BACKTRACE ===---
./sanitizer.so(_Z23get_formatted_backtraceB5cxx11v+0x45) [0x7f99115c7a12]
./sanitizer.so(_ZN13SentinelErrorC1ENSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE+0x5b) [0x7f99115c7b91]
./sanitizer.so(_ZN8Sentinel14add_mutex_lockEim+0x4a6) [0x7f99115bbbce]
./sanitizer.so(pthread_mutex_lock+0x65) [0x7f99115bab78]
examples/c-cycle-example.elf(f2+0x2a) [0x55e16f03d22d]
/usr/lib/libc.so.6(+0x9570a) [0x7f99113ff70a]
/usr/lib/libc.so.6(+0x119aac) [0x7f9911483aac]
---=================---
$
```

As you can see, the sanitizer has detected a potential deadlock, printed
the information regarding it and terminated process.

## How does it work?

The `sanitizer.so` serves as a `pthread` wrapper - it contains several functions that mimic the `pthread` by their name and signature. In other words, it overrides some standard `pthread` functions.

`LD_PRELOAD` allows to load specified shared object(s) before others. When your program calls, say, `pthread_mutex_init(3)`, it will actually call the sanitizer handler. The handler will do internal calculations and then, if everything is alright, will call the actual `pthread_mutex_init(3)` implementation.

## Credits, sources, additional materials

- [ThreadSanitizerDeadlockDetector](https://github.com/google/sanitizers/wiki/ThreadSanitizerDeadlockDetector) by Google - overview of the deadlock detection algorithm
- [A Simple LD_PRELOAD Tutorial, Part Two](https://catonmat.net/simple-ld-preload-tutorial-part-two) by catonmat - a nice, short, quickstart article about `<dlfcn.h>`
- ["How to make backtrace()/backtrace_symbols() print the function names?"](https://stackoverflow.com/a/6934857) - stackoverflow question+answer regarding more readable `backtrace_symbols(3)` output

## About

This project is originally a Multithreaded Programming course assignment at ITMO.
