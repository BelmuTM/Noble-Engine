cmake_minimum_required(VERSION 3.27)
project(NobleEngine LANGUAGES CXX)

# Set C++ standard and output directory
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Static linking the runtime
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")

# Collect source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(NobleEngine ${SOURCES})

# Include source headers
target_include_directories(NobleEngine PRIVATE ${CMAKE_SOURCE_DIR}/src)
# Ensure C++ 23 features
target_compile_features(NobleEngine PRIVATE cxx_std_23)
# Add macro that points to resources folder
target_compile_definitions(NobleEngine PRIVATE RESOURCES_DIR="${CMAKE_SOURCE_DIR}/resources/")

##########################################################
#                       VULKAN SDK                       #
##########################################################

# Vulkan SDK setup
find_package(Vulkan REQUIRED)
target_include_directories(NobleEngine PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(NobleEngine PRIVATE ${Vulkan_LIBRARIES})

#Vulkan debug utils
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
                    -fvk-use-c-layout
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

# json (https://github.com/nlohmann/json)
add_library(json INTERFACE)
target_include_directories(json INTERFACE ${CMAKE_SOURCE_DIR}/external/json)

# STB (https://github.com/nothings/stb)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_SOURCE_DIR}/external/stb)

# tinygltf (https://github.com/syoyo/tinygltf)
add_library(tinygltf INTERFACE)
target_include_directories(tinygltf INTERFACE ${CMAKE_SOURCE_DIR}/external/tinygltf)

# tinyobjloader (https://github.com/tinyobjloader/tinyobjloader)
add_library(tinyobjloader INTERFACE)
target_include_directories(tinyobjloader INTERFACE ${CMAKE_SOURCE_DIR}/external/tinyobjloader)

# Vulkan Memory Allocator (https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
add_library(vma INTERFACE)
target_include_directories(vma INTERFACE ${CMAKE_SOURCE_DIR}/external/vma/include)

# Link libraries to executable
target_link_libraries(NobleEngine PRIVATE
    glfw
    json
    stb
    tinygltf
    tinyobjloader
    vma
)
