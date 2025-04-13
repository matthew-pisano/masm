cmake_minimum_required(VERSION 3.30)
project(mips_cpp)

set(CMAKE_CXX_STANDARD 23)

include_directories(include)

add_executable(mips_cpp src/main.cpp
        include/fileio.h
        src/fileio.cpp)

include(FetchContent)
FetchContent_Declare(
        cli11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG main # or a specific tag/release
)
FetchContent_MakeAvailable(cli11)
target_link_libraries(mips_cpp CLI11::CLI11)