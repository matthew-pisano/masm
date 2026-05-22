set(LIBMASM_SIMULATOR_SOURCES
        cp0.cpp
        cp1.cpp
        cpu.cpp
        heap.cpp
        simulator.cpp
        state.cpp
        syscalls.cpp
)
list(TRANSFORM LIBMASM_SIMULATOR_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
set(LIBMASM_SIMULATOR_SOURCES "${LIBMASM_SIMULATOR_SOURCES}" PARENT_SCOPE)
