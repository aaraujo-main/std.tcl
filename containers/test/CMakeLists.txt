cmake_minimum_required(VERSION 3.16)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(containers_test
    containers_test.cpp
)

target_compile_features(containers_test PRIVATE cxx_std_17)
target_link_libraries(containers_test PRIVATE containers_core gtest gtest_main)
tclxx_link_tcl(containers_test)
containers_enable_o3(containers_test)

add_test(NAME containers_test COMMAND containers_test)

if(TCL_TCLSH)
    set(CONTAINERS_TCLSH ${TCL_TCLSH})
else()
    set(CONTAINERS_TCLSH tclsh)
endif()

add_test(
    NAME containers_load_no_symbol_test
    COMMAND ${CONTAINERS_TCLSH} ${CMAKE_CURRENT_LIST_DIR}/load_no_symbol_test.tcl $<TARGET_FILE:stdcontainers>
)
