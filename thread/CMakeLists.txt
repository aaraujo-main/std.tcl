cmake_minimum_required(VERSION 3.16)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(stdthread LANGUAGES CXX)
endif()

option(STDTHREAD_FORCE_O3 "Compile stdthread targets with -O3" ON)

find_package(TCL QUIET)
find_package(Threads REQUIRED)

set(STDTHREAD_TCLXX_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/../tclxx/include")
if(NOT EXISTS "${STDTHREAD_TCLXX_INCLUDE_DIR}/tclxx.hpp")
    message(FATAL_ERROR "Expected tclxx headers at ${STDTHREAD_TCLXX_INCLUDE_DIR}")
endif()

if(NOT TCL_LIBRARY OR NOT TCL_INCLUDE_PATH)
    find_path(STDTHREAD_TCL_INCLUDE_DIR tcl.h REQUIRED)
    find_library(STDTHREAD_TCL_LIBRARY NAMES tcl tcl8.7 tcl8.6 REQUIRED)
endif()

function(stdthread_link_tcl target)
    if(TARGET Tcl::Tcl)
        target_link_libraries(${target} PRIVATE Tcl::Tcl)
    elseif(TARGET TCL::TCL)
        target_link_libraries(${target} PRIVATE TCL::TCL)
    else()
        if(TCL_INCLUDE_PATH AND TCL_LIBRARY)
            target_include_directories(${target} PRIVATE ${TCL_INCLUDE_PATH})
            target_link_libraries(${target} PRIVATE ${TCL_LIBRARY})
        else()
            target_include_directories(${target} PRIVATE ${STDTHREAD_TCL_INCLUDE_DIR})
            target_link_libraries(${target} PRIVATE ${STDTHREAD_TCL_LIBRARY})
        endif()
    endif()
endfunction()

function(stdthread_enable_o3 target)
    if(NOT STDTHREAD_FORCE_O3)
        return()
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE -O3)
    elseif(MSVC)
        target_compile_options(${target} PRIVATE /O2)
    endif()
endfunction()

set(STDTHREAD_SOURCES
    src/stdthread.cpp
    mutex/stdmutex.cpp
    conditional_variable/stdcond.cpp
)

add_library(stdthread_core STATIC
    ${STDTHREAD_SOURCES}
)

target_compile_features(stdthread_core PUBLIC cxx_std_17)
target_include_directories(stdthread_core
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
    PRIVATE ${STDTHREAD_TCLXX_INCLUDE_DIR}
)
target_link_libraries(stdthread_core PRIVATE Threads::Threads)
stdthread_link_tcl(stdthread_core)
stdthread_enable_o3(stdthread_core)

add_library(stdthread MODULE
    ${STDTHREAD_SOURCES}
)

target_compile_features(stdthread PRIVATE cxx_std_17)
target_include_directories(stdthread
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include
    PRIVATE ${STDTHREAD_TCLXX_INCLUDE_DIR}
)
target_link_libraries(stdthread PRIVATE Threads::Threads)
stdthread_link_tcl(stdthread)
stdthread_enable_o3(stdthread)
set_target_properties(stdthread PROPERTIES PREFIX "" OUTPUT_NAME "stdthread")

if(BUILD_TESTING)
    add_subdirectory(test)
endif()