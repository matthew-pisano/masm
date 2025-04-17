cmake_minimum_required(VERSION 3.30)
project(mips_cpp)

set(CMAKE_CXX_STANDARD 23)

include_directories(include)

add_executable(mipscpp src/main.cpp
        src/fileio.cpp
        src/tokenizer.cpp
        src/parser.cpp
        src/register.cpp
        src/utils.cpp
        src/instruction.cpp
        src/memory.cpp
)

include(FetchContent)
FetchContent_Declare(
        cli11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG main
)
FetchContent_MakeAvailable(cli11)
target_link_libraries(mipscpp CLI11::CLI11)

find_package(Catch2 3 REQUIRED)
add_executable(mipscpp-tests
        test/test_tokenizer.cpp
        test/test_parser.cpp
        src/tokenizer.cpp
        src/parser.cpp
        src/utils.cpp
        src/register.cpp
        src/instruction.cpp
        src/memory.cpp
)
target_link_libraries(mipscpp-tests PRIVATE Catch2::Catch2WithMain)
include(CTest)
include(Catch)
catch_discover_tests(mipscpp-tests)