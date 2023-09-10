cmake_minimum_required (VERSION 3.18.4)
project (ug2tools)

option (UG2TOOLS_BUILD_PRE_UNPACK "Build the pre-unpack executable." ON)
option (UG2TOOLS_BUILD_PRE_PACK "Build the pre-pack executable." ON)

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