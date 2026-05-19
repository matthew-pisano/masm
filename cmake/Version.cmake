# Function to extract value from JSON
function(get_json_value JSON_FILE KEY OUTPUT_VAR)
    file(READ ${JSON_FILE} JSON_CONTENT)

    # Simple regex to extract the value for the given key
    string(REGEX MATCH "\"${KEY}\"[ ]*:[ ]*\"([^\"]+)\"" MATCH_RESULT ${JSON_CONTENT})
    if (MATCH_RESULT)
        set(${OUTPUT_VAR} ${CMAKE_MATCH_1} PARENT_SCOPE)
    else ()
        message(FATAL_ERROR "Could not find key '${KEY}' in ${JSON_FILE}")
    endif ()
endfunction()

# Read version from JSON
get_json_value("${CMAKE_SOURCE_DIR}/version/version.json" "version" PROJECT_VERSION)
message(STATUS "Build ${PROJECT_NAME} version: ${PROJECT_VERSION}")

# Configure the version header
configure_file(
        "${CMAKE_SOURCE_DIR}/version/version.h.in"
        "${CMAKE_BINARY_DIR}/version.h"
        @ONLY
)
