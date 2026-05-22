add_subdirectory(src/assembler)
add_subdirectory(src/simulator)
add_subdirectory(src/io)
add_subdirectory(src/util)

add_library(libmasm STATIC
        ${LIBMASM_ASSEMBLER_SOURCES}
        ${LIBMASM_SIMULATOR_SOURCES}
        ${LIBMASM_IO_SOURCES}
        ${LIBMASM_UTIL_SOURCES}
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
        COMPILE_WARNING_AS_ERROR ON
)
