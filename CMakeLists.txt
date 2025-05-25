cmake_minimum_required(VERSION 3.30)
project(masm)

set(CMAKE_CXX_STANDARD 23)
add_compile_options(-Wall -Wextra -pedantic -Wno-bitwise-op-parentheses)

find_package(pybind11 REQUIRED)

include_directories(include)

add_library(masm STATIC
        src/tokenizer.cpp
        src/parser.cpp
        src/register.cpp
        src/utils.cpp
        src/instruction.cpp
        src/memory.cpp
        src/interpreter.cpp
        src/syscalls.cpp
        src/labels.cpp
        src/directive.cpp
        src/postprocessor.cpp
        src/heap.cpp
        src/consoleio.cpp
)

add_executable(masm-exec src/main.cpp
        src/fileio.cpp
)
target_link_libraries(masm-exec masm)
set_target_properties(masm-exec PROPERTIES OUTPUT_NAME "masm")

add_executable(masm-fg test/fixture_gen.cpp
        src/fileio.cpp
)
target_link_libraries(masm-fg masm)

include(FetchContent)
FetchContent_Declare(
        cli11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG main
)
FetchContent_MakeAvailable(cli11)
target_link_libraries(masm-exec CLI11::CLI11)
target_link_libraries(masm-fg CLI11::CLI11)

find_package(Catch2 3 REQUIRED)
add_executable(masm-tests
        src/fileio.cpp
        test/test_tokenizer.cpp
        test/test_utils.cpp
        test/test_parser.cpp
        src/interpreter.cpp
        src/syscalls.cpp
        test/test_interpreter.cpp
        test/test_syscall.cpp
        test/testing_utilities.cpp
        test/testing_utilities.h
        test/test_arithmetic_instructions.cpp
        test/test_comparison_instructions.cpp
        test/test_control_instructions.cpp
        test/test_memory_instructions.cpp
        test/test_pseudo_instructions.cpp
)
target_link_libraries(masm-tests masm)
target_link_libraries(masm-tests Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(masm-tests)