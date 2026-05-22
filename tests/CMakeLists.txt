FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.13.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(masm-tests
        testing_utilities.cpp
        ${CMAKE_SOURCE_DIR}/mdb/debug_interpreter.cpp
        components/test_intermediates.cpp
        components/test_interpreter.cpp
        components/test_parser.cpp
        components/test_postprocessor.cpp
        components/test_syscall.cpp
        components/test_tokenizer.cpp
        instructions/test_aliased_instructions.cpp
        instructions/test_arithmetic_instructions.cpp
        instructions/test_comparison_instructions.cpp
        instructions/test_control_instructions.cpp
        instructions/test_cp0_instructions.cpp
        instructions/test_cp1_instructions.cpp
        instructions/test_memory_instructions.cpp
        instructions/test_pseudo_instructions.cpp
        utils/test_conversion.cpp
        utils/test_fileio.cpp
)
target_link_libraries(masm-tests PRIVATE libmasm Catch2::Catch2WithMain)
target_include_directories(masm-tests SYSTEM PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(masm-tests PROPERTIES
        COMPILE_WARNING_AS_ERROR ON
)
target_compile_options(masm-tests PRIVATE
        $<$<CXX_COMPILER_ID:Clang>:-Wno-c2y-extensions>
)

include(CTest)
include(Catch)
catch_discover_tests(masm-tests WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
