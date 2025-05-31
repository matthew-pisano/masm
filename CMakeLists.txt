cmake_minimum_required(VERSION 3.30)
project(masm)

set(CMAKE_CXX_STANDARD 23)

add_compile_options(-Wall -Wextra -pedantic -Wno-bitwise-op-parentheses)
include_directories(include)


## MASM Static Library ##

add_library(masm STATIC
        src/io/consoleio.cpp
        src/interpreter/memory.cpp
        src/interpreter/interpreter.cpp
        src/interpreter/syscalls.cpp
        src/interpreter/heap.cpp
        src/interpreter/register.cpp
        src/parser/parser.cpp
        src/parser/instruction.cpp
        src/parser/labels.cpp
        src/parser/directive.cpp
        src/tokenizer/tokenizer.cpp
        src/tokenizer/postprocessor.cpp
        src/utils.cpp
)
target_compile_options(masm PRIVATE -fPIC)


## Python Bindings ##

find_package(pybind11 REQUIRED)
pybind11_add_module(pymasm_core
        src/pybind/python_bindings.cpp
)
# Link the Python module with masm
target_link_libraries(pymasm_core PRIVATE masm)


include(FetchContent)
FetchContent_Declare(
        cli11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG main
)
FetchContent_MakeAvailable(cli11)


## MASM Main Executables ##

add_executable(masm-exec src/main.cpp
        src/io/fileio.cpp
)
target_link_libraries(masm-exec PRIVATE masm CLI11::CLI11)
set_target_properties(masm-exec PROPERTIES OUTPUT_NAME "masm")


## Fixture Generator Executable ##

add_executable(masm-fg test/fixture_gen.cpp
        src/io/fileio.cpp
)
target_link_libraries(masm-fg PRIVATE masm CLI11::CLI11)


## Unit Tests ##

find_package(Catch2 3 REQUIRED)
add_executable(masm-tests
        src/io/fileio.cpp
        test/testing_utilities.cpp
        test/test_components/test_tokenizer.cpp
        test/test_components/test_parser.cpp
        test/test_components/test_interpreter.cpp
        test/test_components/test_syscall.cpp
        test/test_instructions/test_arithmetic_instructions.cpp
        test/test_instructions/test_comparison_instructions.cpp
        test/test_instructions/test_control_instructions.cpp
        test/test_instructions/test_memory_instructions.cpp
        test/test_instructions/test_pseudo_instructions.cpp
        test/test_utils.cpp
)
target_link_libraries(masm-tests PRIVATE masm)
target_link_libraries(masm-tests PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(masm-tests)
