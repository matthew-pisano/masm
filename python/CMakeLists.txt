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

file(READ "${CMAKE_SOURCE_DIR}/version/version.json" VERSION_JSON)
string(JSON VERSION_STRING GET ${VERSION_JSON} version)

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/_version.py.in"
        "${CMAKE_CURRENT_SOURCE_DIR}/pymasm/_version.py"
)

install(TARGETS pymasm_core DESTINATION pymasm COMPONENT python EXCLUDE_FROM_ALL)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/pymasm/_version.py"
        DESTINATION pymasm
        COMPONENT python
        EXCLUDE_FROM_ALL
)
