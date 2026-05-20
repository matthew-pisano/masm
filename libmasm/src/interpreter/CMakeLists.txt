set(LIBMASM_INTERPRETER_SOURCES
        cp0.cpp
        cp1.cpp
        cpu.cpp
        heap.cpp
        interpreter.cpp
        ../assembler/memory.cpp
        state.cpp
        syscalls.cpp
)
list(TRANSFORM LIBMASM_INTERPRETER_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
set(LIBMASM_INTERPRETER_SOURCES "${LIBMASM_INTERPRETER_SOURCES}" PARENT_SCOPE)
