cmake_minimum_required(VERSION 3.22)
project(NobleEngine LANGUAGES CXX)

# C++ standard and output directory
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Collect sources
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(NobleEngine ${SOURCES})

# Include headers
target_include_directories(NobleEngine PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Resources directory path preprocessor
target_compile_definitions(NobleEngine PRIVATE RESOURCES_DIR="${CMAKE_SOURCE_DIR}/resources/")

# Ensure C++ 20 features
target_compile_features(NobleEngine PRIVATE cxx_std_20)

# Statically linking the runtime
target_link_options(
    NobleEngine
    PUBLIC
        -static-libgcc
        -static-libstdc++
)

# Compiler flags
if (MSVC)
    target_compile_options(
        NobleEngine
        PRIVATE
            /W4 /permissive- /wd4100
    )
else()
    target_compile_options(
        NobleEngine
        PRIVATE
            -Wall -Wextra -Wpedantic -Wno-missing-field-initializers -Wno-unused-parameter -Wno-unused-variable
    )
endif()

# Address Sanitizers
# target_compile_options(NobleEngine PRIVATE -fsanitize=address -fno-omit-frame-pointer)
# target_link_options(NobleEngine PRIVATE -fsanitize=address)

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
