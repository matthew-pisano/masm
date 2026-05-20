set(LIBMASM_ASSEMBLER_SOURCES
        directive.cpp
        instruction.cpp
        labels.cpp
        parser.cpp
        serialization.cpp
)
list(TRANSFORM LIBMASM_ASSEMBLER_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
set(LIBMASM_ASSEMBLER_SOURCES "${LIBMASM_ASSEMBLER_SOURCES}" PARENT_SCOPE)
