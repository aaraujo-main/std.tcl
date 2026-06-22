cmake_minimum_required(VERSION 3.16)

find_package(GTest CONFIG QUIET)

if(NOT GTest_FOUND)
    include(FetchContent)

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )

    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

include(GoogleTest)

function(stdthread_add_test target source)
    add_executable(${target}
        ${source}
    )

    target_compile_features(${target} PRIVATE cxx_std_17)
    target_link_libraries(${target} PRIVATE stdthread_core GTest::gtest_main)
    stdthread_link_tcl(${target})
    stdthread_enable_o3(${target})

    gtest_discover_tests(${target})
endfunction()

stdthread_add_test(stdthread_mutex_test mutex_test.cpp)
stdthread_add_test(stdthread_conditional_variable_test conditional_variable_test.cpp)
stdthread_add_test(stdthread_thread_test thread_test.cpp)
