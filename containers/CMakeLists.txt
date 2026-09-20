cmake_minimum_required(VERSION 3.16)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(stdcontainers LANGUAGES CXX)
endif()

include(CTest)
option(CONTAINERS_BUILD_BENCHMARKS "Build containers benchmark binaries" ON)

find_package(TCL QUIET)

if(NOT TCL_LIBRARY OR NOT TCL_INCLUDE_PATH)
    find_path(TCLXX_TCL_INCLUDE_DIR tcl.h REQUIRED)
    find_library(TCLXX_TCL_LIBRARY NAMES tcl tcl8.7 tcl8.6 REQUIRED)
endif()

function(tclxx_link_tcl target)
    if(TARGET Tcl::Tcl)
        target_link_libraries(${target} PRIVATE Tcl::Tcl)
    elseif(TARGET TCL::TCL)
        target_link_libraries(${target} PRIVATE TCL::TCL)
    else()
        if(TCL_INCLUDE_PATH AND TCL_LIBRARY)
            target_include_directories(${target} PRIVATE ${TCL_INCLUDE_PATH})
            target_link_libraries(${target} PRIVATE ${TCL_LIBRARY})
        else()
            target_include_directories(${target} PRIVATE ${TCLXX_TCL_INCLUDE_DIR})
            target_link_libraries(${target} PRIVATE ${TCLXX_TCL_LIBRARY})
        endif()
    endif()
endfunction()

function(containers_enable_o3 target)
    target_compile_options(${target} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/O2>
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-O3>
    )
endfunction()

add_library(containers_core STATIC
    src/containers.cpp
    src/vector.cpp
    src/list.cpp
    src/map.cpp
    src/queue.cpp
    src/stack.cpp
    src/set.cpp
    src/unordered_map.cpp
    src/unordered_set.cpp
)

target_compile_features(containers_core PUBLIC cxx_std_17)
target_include_directories(containers_core PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(containers_core PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../tclxx/include)

tclxx_link_tcl(containers_core)
containers_enable_o3(containers_core)

add_library(stdcontainers MODULE
    src/containers.cpp
    src/vector.cpp
    src/list.cpp
    src/map.cpp
    src/queue.cpp
    src/stack.cpp
    src/set.cpp
    src/unordered_map.cpp
    src/unordered_set.cpp
)

target_compile_features(stdcontainers PRIVATE cxx_std_17)
target_include_directories(stdcontainers PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(stdcontainers PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../tclxx/include)

tclxx_link_tcl(stdcontainers)
containers_enable_o3(stdcontainers)
set_target_properties(stdcontainers PROPERTIES PREFIX "" OUTPUT_NAME "stdcontainers")

if(CONTAINERS_BUILD_BENCHMARKS)
    add_executable(containers_cpp_benchmark benchmarks/cpp_bench.cpp)
    target_compile_features(containers_cpp_benchmark PRIVATE cxx_std_17)
    containers_enable_o3(containers_cpp_benchmark)
endif()

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
