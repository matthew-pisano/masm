cmake_minimum_required(VERSION 3.22)
project(libmasm)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_compile_options(-Wall -Wextra -pedantic)

if (NOT (CMAKE_CROSSCOMPILING AND WIN32))
    add_compile_options(-Wno-bitwise-op-parentheses)
else ()
    add_compile_options(-Wno-parentheses -Wno-strict-aliasing)
endif ()

# Static linking for Windows cross-compilation
if (CMAKE_CROSSCOMPILING AND WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif ()

include(cmake/Version.cmake)

include_directories(include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})


## MASM Static Library ##

add_library(masm STATIC
        libmasm/src/io/consoleio.cpp
        libmasm/src/interpreter/memory.cpp
        libmasm/src/interpreter/interpreter.cpp
        libmasm/src/interpreter/syscalls.cpp
        libmasm/src/interpreter/heap.cpp
        libmasm/src/parser/parser.cpp
        libmasm/src/parser/instruction.cpp
        libmasm/src/parser/labels.cpp
        libmasm/src/parser/directive.cpp
        libmasm/src/tokenizer/tokenizer.cpp
        libmasm/src/tokenizer/postprocessor.cpp
        libmasm/src/utils.cpp
        libmasm/src/interpreter/cpu.cpp
        libmasm/src/interpreter/cp0.cpp
        libmasm/src/interpreter/cp1.cpp
        libmasm/src/interpreter/state.cpp
        libmasm/src/io/streamio.cpp
)

# -fPIC only needed for native Linux builds (not Windows cross-compilation)
if (NOT (CMAKE_CROSSCOMPILING AND WIN32))
    target_compile_options(masm PRIVATE -fPIC)
endif ()


include(FetchContent)
FetchContent_Declare(
        cli11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG main
)
FetchContent_MakeAvailable(cli11)


## MASM Main Executables ##

add_executable(masm-exec masm/masm.cpp
        libmasm/src/io/fileio.cpp
        mdb/intermediates.cpp
        libmasm/src/runtime.cpp
)
target_link_libraries(masm-exec PRIVATE masm CLI11::CLI11)
set_target_properties(masm-exec PROPERTIES OUTPUT_NAME "libmasm")

# Static linking for Windows cross-compilation (ensures dlls are packaged in)
if (CMAKE_CROSSCOMPILING AND WIN32)
    target_link_libraries(masm-exec PRIVATE -static)
endif ()


## MASM Debugger Executables ##

add_executable(masm-dbg mdb/mdb.cpp
        libmasm/src/io/fileio.cpp
        mdb/intermediates.cpp
        mdb/debug_interpreter.cpp
        libmasm/src/runtime.cpp
)
target_link_libraries(masm-dbg PRIVATE masm CLI11::CLI11)
set_target_properties(masm-dbg PROPERTIES OUTPUT_NAME "mdb")

# Static linking for Windows cross-compilation (ensures dlls are packaged in)
if (CMAKE_CROSSCOMPILING AND WIN32)
    target_link_libraries(masm-dbg PRIVATE -static)
endif ()


# No tests or Python bindings for cross-compilation on Windows
if (NOT (CMAKE_CROSSCOMPILING AND WIN32))

    ## Python Bindings ##

    FetchContent_Declare(
            pybind11
            GIT_REPOSITORY https://github.com/pybind/pybind11
            GIT_TAG v2.13.6
    )
    FetchContent_MakeAvailable(pybind11)

    pybind11_add_module(pymasm_core
            python/src/python_bindings.cpp
    )
    # Link the Python module with libmasm
    target_link_libraries(pymasm_core PRIVATE masm)

    ## Fixture Generator Executable ##

    add_executable(masm-fg tools/fixture-gen/fixture_gen.cpp
            libmasm/src/io/fileio.cpp
    )
    target_link_libraries(masm-fg PRIVATE masm CLI11::CLI11)


    ## Unit Tests ##

    FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.8.1
    )
    FetchContent_MakeAvailable(Catch2)

    add_executable(masm-tests
            libmasm/src/runtime.cpp
            mdb/debug_interpreter.cpp
            libmasm/src/io/fileio.cpp
            mdb/intermediates.cpp
            tests/testing_utilities.cpp
            tests/components/test_tokenizer.cpp
            tests/components/test_parser.cpp
            tests/components/test_interpreter.cpp
            tests/components/test_syscall.cpp
            tests/instructions/test_arithmetic_instructions.cpp
            tests/instructions/test_comparison_instructions.cpp
            tests/instructions/test_control_instructions.cpp
            tests/instructions/test_memory_instructions.cpp
            tests/instructions/test_pseudo_instructions.cpp
            tests/instructions/test_aliased_instructions.cpp
            tests/utils/test_utils.cpp
            tests/instructions/test_cp0_instructions.cpp
            tests/instructions/test_cp1_instructions.cpp
            tests/components/test_intermediates.cpp
            tests/utils/test_fileio.cpp
    )
    target_link_libraries(masm-tests PRIVATE masm)
    target_link_libraries(masm-tests PRIVATE Catch2::Catch2WithMain)

    include(CTest)
    include(Catch)
    catch_discover_tests(masm-tests
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )

endif ()
