cmake_minimum_required(VERSION 3.16)

project(stdtcl LANGUAGES CXX)

include(GNUInstallDirs)
include(CTest)

set(STD_VERSION "0.1.0")
set(STD_PACKAGE_DIR "std${STD_VERSION}" CACHE STRING "Package directory under CMAKE_INSTALL_LIBDIR")

find_package(TCL QUIET)
find_package(Threads REQUIRED)

if(NOT TCL_LIBRARY OR NOT TCL_INCLUDE_PATH)
    find_path(STD_TCL_INCLUDE_DIR tcl.h REQUIRED)
    find_library(STD_TCL_LIBRARY NAMES tcl tcl8.7 tcl8.6 REQUIRED)
endif()

function(std_link_tcl target)
    if(TARGET Tcl::Tcl)
        target_link_libraries(${target} PRIVATE Tcl::Tcl)
    elseif(TARGET TCL::TCL)
        target_link_libraries(${target} PRIVATE TCL::TCL)
    else()
        if(TCL_INCLUDE_PATH AND TCL_LIBRARY)
            target_include_directories(${target} PRIVATE ${TCL_INCLUDE_PATH})
            target_link_libraries(${target} PRIVATE ${TCL_LIBRARY})
        else()
            target_include_directories(${target} PRIVATE ${STD_TCL_INCLUDE_DIR})
            target_link_libraries(${target} PRIVATE ${STD_TCL_LIBRARY})
        endif()
    endif()
endfunction()

add_subdirectory(containers)
add_subdirectory(thread)

set_target_properties(containers_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(stdthread_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(std MODULE
    std.cpp
)

target_compile_features(std PRIVATE cxx_std_17)
target_include_directories(std PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/containers/include
    ${CMAKE_CURRENT_LIST_DIR}/thread/include
)
target_link_libraries(std PRIVATE
    containers_core
    stdthread_core
    Threads::Threads
)
std_link_tcl(std)

set_target_properties(std PROPERTIES
    PREFIX ""
    OUTPUT_NAME "std"
)

install(TARGETS std
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${STD_PACKAGE_DIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/${STD_PACKAGE_DIR}
)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/pkgIndex.tcl
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/${STD_PACKAGE_DIR}
)
