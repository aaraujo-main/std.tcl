# std::thread Tcl Extension

A C++17 Tcl extension that exposes essential C++ standard threading primitives directly into Tcl, with minimal abstraction overhead.

## Overview

This package provides:
- `std::mutex` object handles and manual lock APIs
- `std::scoped_lock` scoped lock handles for one or more mutexes
- `std::conditional_variable` (plus alias `std::coditional_variable`) condition variable APIs
- script-backed `std::thread` spawn/join/detach/swap
- `std::this_thread` static utilities (`get_id`, `sleep_for`)

Arguments provided to threads have their internal value copied into the new interpreter of each thread holding it and it is held by a standalone object. If the `Tcl_Obj` contains a `std::shared_ptr`, and the `Tcl_ObjType` points to a `tclxx::ObjType` class with `shared` ownership, the internal representation will be shared across across different interpreters via internal pointee of `std::shared_ptr`. 

## Requirements

| Component | Version |
|-----------|---------|
| C++ compiler | C++17 or later |
| CMake | 3.16 or later |
| Tcl | 8.6 or later |

## Build

### Standalone build from `thread/`

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Artifacts:
- `build/stdthread.so` - loadable Tcl module
- `build/libstdthread_core.a` - static library for C++ tests/embedding

## Tests

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build -j
ctest --test-dir build --output-on-failure
```

## Loading in Tcl

```tcl
load ./stdthread.so
# or, if installed and indexed
package require std::thread
```

`Stdthread_Init` initializes:
- `Stdmutex_Init`
- `Stdcond_Init`
- thread + this_thread commands

## API Reference

## `::std::mutex`

### `::std::mutex::new.shared.shared`
Creates a new `std::mutex` handle, which is shareable across different threads.

Example:
```tcl
set m [::std::mutex::new.shared]
```

### `::std::mutex::lock mutex`
Locks the mutex.

### `::std::mutex::try_lock mutex`
Attempts to lock the mutex; returns Tcl boolean.

### `::std::mutex::unlock mutex`
Unlocks the mutex.

## `::std::mutex::guard`

### `::std::mutex::guard::new mutex`
Creates a guard object that locks on construction.

### `::std::mutex::guard::release guard`
Releases the guard lock early; returns Tcl boolean indicating whether a lock was released.

## `::std::scoped_lock`

### `::std::scoped_lock::new.shared(args) mutex1 ?mutex2 ...?`
Creates a scoped lock object that locks all listed mutexes and unlocks on object destruction.

Example:
```tcl
set a [::std::mutex::new.shared]
set b [::std::mutex::new.shared]
set sl [::std::scoped_lock::new.shared(args) $a $b]
unset sl
```

## `::std::conditional_variable` and `::std::coditional_variable`

### `::std::conditional_variable::new.shared`
Creates a new condition variable handle.

### `::std::conditional_variable::wait condVar guard`
Waits on the condition variable using a `std::mutex_guard` handle.

### `::std::conditional_variable::wait_for condVar guard milliseconds`
Waits with timeout; returns Tcl boolean (`1` for notified, `0` for timeout).

### `::std::conditional_variable::notify_one condVar`
Notifies one waiting thread.

### `::std::conditional_variable::notify_all condVar`
Notifies all waiting threads.

## `::std::thread`

### `::std::thread::new func ?arg1 arg2 ...?`
Spawns a C++ thread running Tcl `body` from apply-style `func`:
- `func` must be `{t_args body}`, where:
    - `t_args`: a list of `{?type? arg}`, where:
        - `type`: a optional type (`int`, `double`, `string`, `bool`). By default, it is a `std:shared_ptr`.
        - `arg`: a variable name
    - `body`: a body script
- The core value of the each argument is copied into the heap, then copied to the Tcl_Obj into the interpreter of the thread according to their type.

Example:
```tcl
set v [std::vector::new.shared(args) {1 2 3}]
set flag 1
set t [::std::thread::new {{v {bool flag}} {
    if {$flag} {
        std::vector::push v 4
    }
}} $v $flag]
::std::thread::join $t
puts $v; # 1 2 3 4
```

### `::std::thread::join thread`
Joins a joinable thread.

### `::std::thread::detach thread`
Detaches a joinable thread.

### `::std::thread::swap threadVar1 threadVar2`
Swaps underlying thread objects between two Tcl variables.

## `::std::this_thread`

### `::std::this_thread::get_id`
Returns the current thread id as a string.

### `::std::this_thread::sleep_for milliseconds`
Sleeps for `milliseconds` and returns the current thread id.

## Notes

- Handle string representations are decimal pointer addresses with no prefixes or suffixes.
- Parsing an arbitrary address with `SetFromAny` results in a new object of that type.

## License

MIT License. See the workspace license file policy used by sibling modules.
