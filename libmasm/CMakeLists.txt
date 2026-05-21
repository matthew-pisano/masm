add_subdirectory(src/assembler)
add_subdirectory(src/interpreter)
add_subdirectory(src/io)

add_library(libmasm STATIC
        ${LIBMASM_ASSEMBLER_SOURCES}
        ${LIBMASM_INTERPRETER_SOURCES}
        ${LIBMASM_IO_SOURCES}
        src/utils.cpp
)
target_include_directories(libmasm
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_target_properties(libmasm PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        OUTPUT_NAME "masm"
        POSITION_INDEPENDENT_CODE ON
)
