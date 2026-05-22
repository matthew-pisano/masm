set(MDB_SOURCES
        debug_simulator.cpp
)

add_executable(mdb mdb.cpp ${MDB_SOURCES})
target_link_libraries(mdb PRIVATE libmasm CLI11::CLI11)
target_include_directories(mdb SYSTEM PRIVATE
        ${CMAKE_BINARY_DIR}
        ${CMAKE_SOURCE_DIR}/shared
)
set_target_properties(mdb PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        COMPILE_WARNING_AS_ERROR ON
)