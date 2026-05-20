set(LIBMASM_PARSER_SOURCES
        directive.cpp
        instruction.cpp
        labels.cpp
        parser.cpp
)
list(TRANSFORM LIBMASM_PARSER_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
set(LIBMASM_PARSER_SOURCES "${LIBMASM_PARSER_SOURCES}" PARENT_SCOPE)
