set(CPACK_PACKAGE_NAME "masm")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "Matthew Pisano")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A MIPS Assembler and Simulator")
set(CPACK_PACKAGE_DESCRIPTION "The masm MIPS assembler, simulator, and debugger.")
set(CUSTOM_PACKAGE_URL "https://github.com/matthew-pisano/masm")
set(CPACK_GENERATOR "RPM;DEB;TGZ")

# === RPM Package Generation === #

set(CPACK_RPM_PACKAGE_LICENSE "BSD-3-Clause")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RPM_PACKAGE_GROUP "Development/Languages")
set(CPACK_RPM_SPEC_MORE_DEFINE "%define _buildhost anonymized")
set(CPACK_RPM_PACKAGE_URL "${CUSTOM_PACKAGE_URL}")
set(CPACK_RPM_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION}") # RPM does not automatically pick up the cpack description

# === DEB Package Generation === #

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR}")
set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${CUSTOM_PACKAGE_URL}")

include(CPack)