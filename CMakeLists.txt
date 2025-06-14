cmake_minimum_required(VERSION 3.30)
project(masm)

set(CMAKE_CXX_STANDARD 23)


# Function to extract value from JSON
function(get_json_value JSON_FILE KEY OUTPUT_VAR)
    file(READ ${JSON_FILE} JSON_CONTENT)

    # Simple regex to extract the value for the given key
    string(REGEX MATCH "\"${KEY}\"[ ]*:[ ]*\"([^\"]+)\"" MATCH_RESULT ${JSON_CONTENT})
    if (MATCH_RESULT)
        set(${OUTPUT_VAR} ${CMAKE_MATCH_1} PARENT_SCOPE)
    else ()
        message(FATAL_ERROR "Could not find key '${KEY}' in ${JSON_FILE}")
    endif ()
endfunction()


# Read version from JSON
get_json_value("${CMAKE_CURRENT_SOURCE_DIR}/version.json" "version" PROJECT_VERSION)
message(STATUS "Build ${PROJECT_NAME} version: ${PROJECT_VERSION}")

# Configure the version header
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/version.h"
        @ONLY
)

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

include_directories(include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})


## MASM Static Library ##

add_library(masm STATIC
        src/io/consoleio.cpp
        src/interpreter/memory.cpp
        src/interpreter/interpreter.cpp
        src/interpreter/syscalls.cpp
        src/interpreter/heap.cpp
        src/parser/parser.cpp
        src/parser/instruction.cpp
        src/parser/labels.cpp
        src/parser/directive.cpp
        src/tokenizer/tokenizer.cpp
        src/tokenizer/postprocessor.cpp
        src/utils.cpp
        src/interpreter/cpu.cpp
        src/interpreter/cp0.cpp
        src/interpreter/cp1.cpp
        src/interpreter/state.cpp
        src/io/streamio.cpp
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

add_executable(masm-exec src/main.cpp
        src/io/fileio.cpp
)
target_link_libraries(masm-exec PRIVATE masm CLI11::CLI11)
set_target_properties(masm-exec PROPERTIES OUTPUT_NAME "masm")

# Static linking for Windows cross-compilation (ensures dlls are packaged in)
if (CMAKE_CROSSCOMPILING AND WIN32)
    target_link_libraries(masm-exec PRIVATE -static)
endif ()


if (NOT (CMAKE_CROSSCOMPILING AND WIN32))

    ## Python Bindings ##

    find_package(pybind11 REQUIRED)
    pybind11_add_module(pymasm_core
            src/pybind/python_bindings.cpp
    )
    # Link the Python module with masm
    target_link_libraries(pymasm_core PRIVATE masm)

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
            test/test_instructions/test_cp0_instructions.cpp
            test/test_instructions/test_cp1_instructions.cpp
    )
    target_link_libraries(masm-tests PRIVATE masm)
    target_link_libraries(masm-tests PRIVATE Catch2::Catch2WithMain)

    include(CTest)
    include(Catch)
    catch_discover_tests(masm-tests)

endif ()
