cmake_minimum_required(VERSION 3.22)
project(masm)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_compile_options(-Wall -Wextra -pedantic)

include(FetchContent)
FetchContent_Declare(
        cli11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG main
)
FetchContent_MakeAvailable(cli11)

include(cmake/Version.cmake)

# Static linking for Windows cross-compilation
if (CMAKE_CROSSCOMPILING AND WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif ()

add_subdirectory(libmasm)
add_subdirectory(masm)
add_subdirectory(mdb)
add_subdirectory(msim)

# No tests or Python bindings for cross-compilation on Windows
if (NOT (CMAKE_CROSSCOMPILING AND WIN32))
    add_subdirectory(python)
    add_subdirectory(tests)
    add_subdirectory(tools)
endif ()
