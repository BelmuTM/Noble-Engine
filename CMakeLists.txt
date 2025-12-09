include(FetchContent)

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

# Static linking the runtime
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
else ()
    target_compile_options(
        NobleEngine
        PRIVATE
            -Wall -Wextra -Wpedantic -Wno-missing-field-initializers -Wno-unused-parameter -Wno-unused-variable
    )
endif ()

# Address Sanitizers
# target_compile_options(NobleEngine PRIVATE -fsanitize=address -fno-omit-frame-pointer)
# target_link_options(NobleEngine PRIVATE -fsanitize=address)

##########################################################
#                         VULKAN                         #
##########################################################

# Vulkan-Headers (https://github.com/KhronosGroup/Vulkan-Headers)
add_subdirectory(thirdparty/vulkan-headers)

target_include_directories(NobleEngine SYSTEM PRIVATE thirdparty/vulkan-headers/include)

target_link_libraries(NobleEngine PRIVATE vulkan)

# Determine if the Vulkan SDK is installed on the system
message(CHECK_START "Check for Vulkan SDK")

find_package(Vulkan QUIET)

if (Vulkan_FOUND)
    message(CHECK_PASS "done")
    target_compile_definitions(NobleEngine PRIVATE VULKAN_SDK)
else ()
    message(CHECK_FAIL "failed (validation layers will be disabled!)")
endif ()

# Vulkan debug utils
target_compile_definitions(NobleEngine PRIVATE $<$<CONFIG:Debug>:VULKAN_DEBUG_UTILS>)

##########################################################
#                         SLANG                          #
##########################################################

include("cmake/slang.cmake")

add_dependencies(NobleEngine SlangShaders)

##########################################################
#             THIRD-PARTY UTILITY LIBRARIES              #
##########################################################

# glfw (https://github.com/glfw/glfw)
add_subdirectory(thirdparty/glfw)

# glm (https://github.com/g-truc/glm)
add_subdirectory(thirdparty/glm)

target_compile_definitions(
    glm
    INTERFACE
        GLM_FORCE_CXX20
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_ENABLE_EXPERIMENTAL
)

# SPIRV-Headers (https://github.com/KhronosGroup/SPIRV-Headers)
set(SPIRV-HEADERS_Libraries "${CMAKE_SOURCE_DIR}/thirdparty/spirv-headers/include")

# SPIRV-Reflect (https://github.com/KhronosGroup/SPIRV-Reflect)
add_library(spirv-reflect STATIC thirdparty/spirv-reflect/spirv_reflect.cpp)

target_include_directories(spirv-reflect SYSTEM PRIVATE ${SPIRV-HEADERS_Libraries})

target_compile_definitions(spirv-reflect PUBLIC SPIRV_REFLECT_USE_SYSTEM_SPIRV_H)

# Link compiled libraries to executable
target_link_libraries(
    NobleEngine
    PRIVATE
        glfw
        glm
        spirv-reflect
)

# json (https://github.com/nlohmann/json)
# STB (https://github.com/nothings/stb)
# tinygltf (https://github.com/syoyo/tinygltf)
# tinyobjloader (https://github.com/tinyobjloader/tinyobjloader)
# Vulkan Memory Allocator (https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
target_include_directories(
    NobleEngine
    SYSTEM
    PRIVATE
        ${CMAKE_SOURCE_DIR}/thirdparty
        ${CMAKE_SOURCE_DIR}/thirdparty/json
        ${CMAKE_SOURCE_DIR}/thirdparty/stb
        ${SPIRV-HEADERS_Libraries}
)
