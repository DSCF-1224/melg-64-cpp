cmake_minimum_required(VERSION 3.14)
project(melg64 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(melg64 INTERFACE)
target_include_directories(melg64 INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

option(MELG64_BUILD_TESTS "Build tests" ON)
if(MELG64_BUILD_TESTS)
    enable_testing()

    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/build_info.inc
        ${CMAKE_CURRENT_BINARY_DIR}/tests/build_info.hpp
    )

    add_executable(test_melg64 tests/test_melg64.cpp)
    target_link_libraries(test_melg64 PRIVATE melg64)
    target_include_directories(test_melg64 PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/tests
    )

    add_test(
        NAME test_melg64
        COMMAND test_melg64
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    foreach(variant 607 1279 2281)
        add_executable(test_melg${variant} tests/test_melg${variant}.cpp)
        target_link_libraries(test_melg${variant} PRIVATE melg64)
        target_include_directories(test_melg${variant} PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}/tests
        )

        add_test(
            NAME test_melg${variant}
            COMMAND test_melg${variant}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endforeach()
endif()
