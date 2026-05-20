add_executable(masm masm.cpp)
target_link_libraries(masm PRIVATE libmasm CLI11::CLI11)
target_include_directories(masm SYSTEM PRIVATE
        ${CMAKE_BINARY_DIR}
)
set_target_properties(masm PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        COMPILE_WARNING_AS_ERROR ON
)
