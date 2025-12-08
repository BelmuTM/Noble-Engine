cmake_minimum_required(VERSION 3.27)
project(NobleEngine LANGUAGES CXX)

# Set C++ standard and output directory
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Static linking the runtime
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc")

# Collect source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")
add_executable(NobleEngine ${SOURCES})

# Include source headers
target_include_directories(NobleEngine PRIVATE ${CMAKE_SOURCE_DIR}/src)
# Add macro that points to resources folder
target_compile_definitions(NobleEngine PRIVATE RESOURCES_DIR="${CMAKE_SOURCE_DIR}/resources/")

# Ensure C++ 23 features
target_compile_features(NobleEngine PRIVATE cxx_std_23)

# Set compiler flags
# target_compile_options(NobleEngine PRIVATE
#     # MSVC
#     $<$<CXX_COMPILER_ID:MSVC>:
#     /W4 /permissive- /wd4100>
#     # GCC-based compilers
#     $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:
#     -Wall -Wextra -Wpedantic -Wno-missing-field-initializers -Wno-unused-parameter -Wno-unused-variable>
# )

# target_compile_options(NobleEngine PRIVATE -fsanitize=address -fno-omit-frame-pointer)
# target_link_options(NobleEngine PRIVATE -fsanitize=address)

##########################################################
#                         VULKAN                         #
##########################################################

# Vulkan-Headers (https://github.com/KhronosGroup/Vulkan-Headers)
add_subdirectory(external/vulkan-headers)

target_include_directories(NobleEngine PRIVATE external/vulkan-headers/include)

target_link_libraries(NobleEngine PRIVATE vulkan)

# Vulkan debug utils
target_compile_definitions(NobleEngine PRIVATE $<$<CONFIG:Debug>:VULKAN_DEBUG_UTILS>)

##########################################################
#                  SHADERS COMPILATION                   #
##########################################################

set(SHADERS_DIR_RELATIVE resources/shaders)
set(SHADERS_DIR ${CMAKE_SOURCE_DIR}/${SHADERS_DIR_RELATIVE})

function(add_slang_shader_target TARGET)
    cmake_parse_arguments(SHADERS "" "" "SOURCES;ENTRIES" ${ARGN})

    set(SHADERS_SPV_DIR_RELATIVE ${SHADERS_DIR_RELATIVE}/spv)
    set(SHADERS_SPV_DIR ${SHADERS_DIR}/spv)
    file(MAKE_DIRECTORY ${SHADERS_SPV_DIR})

    set(SPV_OUTPUTS "")

    foreach(SOURCE ${SHADERS_SOURCES})
        cmake_path(GET SOURCE STEM SOURCE_NAME)

        foreach(ENTRY_PAIR ${SHADERS_ENTRIES})
            string(REPLACE "=" ";" ENTRY_SPLIT "${ENTRY_PAIR}")
            list(GET ENTRY_SPLIT 0 ENTRY_POINT)
            list(GET ENTRY_SPLIT 1 STAGE_NAME )

            string(SUBSTRING "${STAGE_NAME}" 0 4 STAGE_NAME_SHORT)

            set(SPV_FILE_NAME ${SOURCE_NAME}.${STAGE_NAME_SHORT}.spv)
            set(SPV_FILE ${SHADERS_SPV_DIR}/${SPV_FILE_NAME})
            list(APPEND SPV_OUTPUTS ${SPV_FILE})

            # Run Slangc command
            add_custom_command(
                OUTPUT ${SPV_FILE}
                COMMAND slangc ${SOURCE}
                    -target spirv
                    -entry ${ENTRY_POINT}
                    -stage ${STAGE_NAME}
                    -profile spirv_1_4
                    -emit-spirv-directly
                    -fvk-use-entrypoint-name
                    -o ${SPV_FILE}
                DEPENDS ${SOURCE}
                COMMENT "Building SPIR-V object ${SHADERS_SPV_DIR_RELATIVE}/${SPV_FILE_NAME}"
                VERBATIM
            )
        endforeach()
    endforeach()

    add_custom_target(${TARGET} DEPENDS ${SPV_OUTPUTS})
endfunction()

file(GLOB_RECURSE SHADERS_SOURCES ${SHADERS_DIR}/src/*.slang)
add_slang_shader_target(ShadersModule
    SOURCES ${SHADERS_SOURCES}
    ENTRIES "fragMain=fragment" "vertMain=vertex"
)

add_dependencies(NobleEngine ShadersModule)

##########################################################
#             THIRD-PARTY UTILITY LIBRARIES              #
##########################################################

# glfw (https://github.com/glfw/glfw)
add_subdirectory(external/glfw)

# glm (https://github.com/g-truc/glm)
add_subdirectory(external/glm)

target_compile_definitions(glm INTERFACE
    GLM_FORCE_CXX20
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLM_ENABLE_EXPERIMENTAL
)

# SPIRV-Headers (https://github.com/KhronosGroup/SPIRV-Headers)
add_subdirectory(external/spirv-headers)

# SPIRV-Reflect (https://github.com/KhronosGroup/SPIRV-Reflect)
add_library(spirv-reflect STATIC external/spirv-reflect/spirv_reflect.cpp)

target_include_directories(spirv-reflect PUBLIC
    ${CMAKE_SOURCE_DIR}/external
    ${SPIRV-Headers_SOURCE_DIR}/include
)

target_compile_definitions(spirv-reflect PUBLIC SPIRV_REFLECT_USE_SYSTEM_SPIRV_H)

# Link compiled libraries to executable
target_link_libraries(NobleEngine PRIVATE
    glfw
    glm
    spirv-reflect
)

# json (https://github.com/nlohmann/json)
# STB (https://github.com/nothings/stb)
# tinygltf (https://github.com/syoyo/tinygltf)
# tinyobjloader (https://github.com/tinyobjloader/tinyobjloader)
# Vulkan Memory Allocator (https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
target_include_directories(NobleEngine PRIVATE
    ${CMAKE_SOURCE_DIR}/external
    ${CMAKE_SOURCE_DIR}/external/json
    ${CMAKE_SOURCE_DIR}/external/stb
)
