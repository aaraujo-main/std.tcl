# std::containers

std::containers provides Tcl command bindings for selected C++ standard-style
containers, implemented with tclxx and exposed as ::std::* command families.

The Tcl package name is std::containers.

## Features

- C++17 implementation.
- Split public headers by container type under include/containers/.
- Explicit Tcl_Obj* ownership for standard containers.
- Reserve support for growth-sensitive vector and unordered_map variants.
- CTest-based test coverage for the standard command families.

## Build

```bash
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build -j
ctest --test-dir build --output-on-failure
```

The module binary is build/stdcontainers.so.

## Load In Tcl

```tcl
load ./stdcontainers.so
# or
package require std::containers
```

## Standard Command Families

- ::std::vector::*
- ::std::list::*
- ::std::stack::*
- ::std::set::*
- ::std::unordered_map::*

These retain prior command names:

- vector: new size empty at set list reserve push pop clear
- list: new size empty at list push pop clear
- stack: new size empty top push pop clear
- set: new size empty contains list insert erase clear
- unordered_map: new size empty exists get keys reserve put erase clear

## Examples

```tcl
set v [::std::vector::new]
::std::vector::push v a b c
puts [::std::vector::at $v 1]
```

## Header Layout

- include/containers/containers.hpp (umbrella include)
- include/containers/vector.hpp
- include/containers/list.hpp
- include/containers/stack.hpp
- include/containers/set.hpp
- include/containers/unordered_map.hpp

## Benchmarks

Files in benchmarks/:

- becnhmarks.tcl
- cpp_bench.cpp
- python_bench.py

Run Tcl benchmark:

```bash
tclsh benchmarks/becnhmarks.tcl --lib build/stdcontainers.so
```

Run C++ benchmark:

```bash
cmake --build build -j --target containers_cpp_benchmark
./build/containers_cpp_benchmark
```
