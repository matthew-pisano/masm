FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11
        GIT_TAG v3.0.4
)
FetchContent_MakeAvailable(pybind11)

pybind11_add_module(pymasm_core
        src/python_bindings.cpp
)
# Link the Python module with libmasm
target_link_libraries(pymasm_core PRIVATE libmasm)

# Copy shared object to python folder for packaging
add_custom_command(
        TARGET pymasm_core
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:pymasm_core>"
        "${CMAKE_SOURCE_DIR}/python/pymasm/"
        COMMENT "Copy shared object to python folder for packaging"
)
