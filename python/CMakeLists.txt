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

install(TARGETS pymasm_core DESTINATION pymasm COMPONENT python EXCLUDE_FROM_ALL)
