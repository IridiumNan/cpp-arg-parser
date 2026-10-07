cmake_minimum_required(VERSION 3.14)
project(arg_parser VERSION 1.0.0 LANGUAGES CXX)

# Header-only interface library.
add_library(arg_parser INTERFACE)
add_library(arg_parser::arg_parser ALIAS arg_parser)

target_include_directories(arg_parser INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(arg_parser INTERFACE cxx_std_17)

# Options.
option(ARG_PARSER_BUILD_TESTS    "Build tests"    ON)
option(ARG_PARSER_BUILD_EXAMPLES "Build examples" ON)

# Tests.
if(ARG_PARSER_BUILD_TESTS)
    enable_testing()

    add_executable(test_arg_parser test_arg_parser.cpp)
    target_link_libraries(test_arg_parser PRIVATE arg_parser::arg_parser)
    add_test(NAME test_arg_parser COMMAND test_arg_parser)
endif()

# Examples.
if(ARG_PARSER_BUILD_EXAMPLES)
    add_executable(example example.cpp)
    target_link_libraries(example PRIVATE arg_parser::arg_parser)

    add_executable(best_practice best_practice.cpp)
    target_link_libraries(best_practice PRIVATE arg_parser::arg_parser)

    # On Windows with MinGW, wmain requires -municode.
    if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_options(example PRIVATE -municode)
        target_link_options(best_practice PRIVATE -municode)
    endif()
endif()

# Put executables under build/bin/.
set_target_properties(example best_practice test_arg_parser
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
