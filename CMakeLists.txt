cmake_minimum_required(VERSION 3.22)
project(NobleEngine LANGUAGES CXX C)

# ---- C++ standard ----------------------------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Output Directories ----------------------------------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# ---- Sources ---------------------------------------------------------------------------------------------------------
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
add_executable(NobleEngine ${SOURCES})

# Ensure C++ 20 features
target_compile_features(NobleEngine PRIVATE cxx_std_20)

# ---- Include Paths ---------------------------------------------------------------------------------------------------
target_include_directories(NobleEngine PRIVATE ${CMAKE_SOURCE_DIR}/src)

# ---- Compile Definitions ---------------------------------------------------------------------------------------------
target_compile_definitions(NobleEngine PRIVATE
    RESOURCES_DIR="${CMAKE_SOURCE_DIR}/resources/"
    $<$<CONFIG:Debug>:NOBLE_DEBUG_UTILS>
    # $<$<CONFIG:Debug>:LOG_FILE_WRITE>
)

# ---- Static Runtime Linkage ------------------------------------------------------------------------------------------
# Not applied on MSVC
if (NOT MSVC)
    target_link_options(NobleEngine PRIVATE
        -static-libgcc
        -static-libstdc++
    )
endif()

# MSVC static runtime (Debug = MTd, Release = MT)
set_target_properties(NobleEngine PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

# ---- Compiler Warnings -----------------------------------------------------------------------------------------------
# Sanitizers are injected via CMakePresets.json, not here

if (MSVC)

    target_compile_options(NobleEngine PRIVATE
        /W4
        /permissive-
        /w14505         # Unreferenced local function
        /wd4100         # Unreferenced formal parameter (-Wno-unused-parameter)
    )

else()

    target_compile_options(NobleEngine PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wno-missing-field-initializers
        -Wno-unused-parameter
        -Wno-unused-variable
    )

endif()

# ---- Interprocedural Optimization ------------------------------------------------------------------------------------
include(CheckIPOSupported)

check_ipo_supported(RESULT _ipo_supported OUTPUT _ipo_output)

if (_ipo_supported)
    set_target_properties(NobleEngine PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE ON
    )
endif()

##########################################################
#                         VULKAN                         #
##########################################################

include("cmake/vulkan.cmake")

setup_vulkan(NobleEngine)

##########################################################
#                         SLANG                          #
##########################################################

include("cmake/slang.cmake")

fetch_slang(NobleEngine)
compile_slang(NobleEngine)

##########################################################
#             THIRD-PARTY UTILITY LIBRARIES              #
##########################################################

include("cmake/thirdparty.cmake")

setup_thirdparty(NobleEngine)
