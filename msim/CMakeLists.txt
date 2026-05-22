add_executable(msim msim.cpp)
target_link_libraries(msim PRIVATE libmasm CLI11::CLI11)
target_include_directories(msim SYSTEM PRIVATE
        ${CMAKE_BINARY_DIR}
        ${CMAKE_SOURCE_DIR}/shared
)
set_target_properties(msim PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        COMPILE_WARNING_AS_ERROR ON
)
