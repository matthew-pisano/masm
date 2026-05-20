add_executable(masm-fg masm.cpp)
target_link_libraries(masm-fg PRIVATE libmasm CLI11::CLI11)
set_target_properties(masm-fg PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        COMPILE_WARNING_AS_ERROR ON
)
