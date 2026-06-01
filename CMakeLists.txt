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

    foreach(variant 607 1279 2281 4253 11213 19937 44497)
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

        set_tests_properties(test_melg${variant} PROPERTIES LABELS "test")
    endforeach()
endif()

option(MELG64_BUILD_EXAMPLES "Build examples" ON)
if(MELG64_BUILD_EXAMPLES)
    enable_testing()

    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/build_info.inc
        ${CMAKE_CURRENT_BINARY_DIR}/examples/build_info.hpp
    )

    foreach(variant 607 1279 2281 4253 11213 19937 44497)
        add_executable(basic_usage_melg${variant} examples/basic_usage_melg${variant}.cpp)
        target_link_libraries(basic_usage_melg${variant} PRIVATE melg64)
        target_include_directories(basic_usage_melg${variant} PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}/examples
        )

        add_test(
            NAME basic_usage_melg${variant}
            COMMAND basic_usage_melg${variant}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

        set_tests_properties(basic_usage_melg${variant} PROPERTIES LABELS "example")
    endforeach()
endif()
