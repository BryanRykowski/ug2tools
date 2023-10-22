cmake_minimum_required (VERSION 3.18.4)
project (ug2tools VERSION 0.2.0)
set (UG2TOOLS_PACKAGE_RELEASE_NUM 1)

option (UG2TOOLS_PACKAGE_RPM "Generate a x86_64 rpm using cpack." OFF)
option (UG2TOOLS_BUILD_PRE_UNPACK "Build the pre-unpack executable." ON)
option (UG2TOOLS_BUILD_PRE_PACK "Build the pre-pack executable." ON)
option (UG2TOOLS_BUILD_TEX2DDS "Build the tex2dds executable." ON)
option (UG2TOOLS_BUILD_DDS2TEX "Build the dds2tex executable." ON)

if (MSVC)
    add_compile_options (/W4)
    else ()
    add_compile_options (-Wall -Wextra)
    endif ()
    
if (UG2TOOLS_BUILD_PRE_UNPACK)
    add_subdirectory (pre-unpack)
endif ()
    
if (UG2TOOLS_BUILD_PRE_PACK)
    add_subdirectory (pre-pack)
endif ()

if (UG2TOOLS_BUILD_TEX2DDS)
    add_subdirectory (tex2dds)
endif ()

if (UG2TOOLS_BUILD_DDS2TEX)
    add_subdirectory (dds2tex)
endif ()
    
if (UG2TOOLS_PACKAGE_RPM)
    set (CPACK_GENERATOR "RPM")

    set (CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/BryanRykowski/ug2tools")
    set (CPACK_PACKAGE_CONTACT "90996715+BryanRykowski@users.noreply.github.com")
    set (CPACK_PACKAGE_DESCRIPTION "A collection of tools for modding THUG 2/THUG Pro.")
    
    set (CPACK_RPM_PACKAGE_RELEASE ${UG2TOOLS_PACKAGE_RELEASE_NUM})
    set (CPACK_RPM_PACKAGE_LICENSE "MIT")
    set (CPACK_RPM_PACKAGE_REQUIRES "glibc, libstdc++, libgcc")
    set (CPACK_RPM_PACKAGE_DESCRIPTION ${CPACK_PACKAGE_DESCRIPTION})
    set (CPACK_RPM_COMPRESSION_TYPE "gzip")
    # Why do I have to select the default. It should be the DEFAULT.
    set (CPACK_RPM_FILE_NAME RPM-DEFAULT)
    set (CPACK_RPM_CHANGELOG_FILE ${CMAKE_SOURCE_DIR}/CHANGELOG)

    include(CPack)
endif()