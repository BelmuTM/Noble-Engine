cmake_minimum_required(VERSION 3.27)
project(NobleEngine LANGUAGES CXX)

# Set C++ standard and output directory
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Collect source files and add Windows resource file
file(GLOB_RECURSE CPP_SOURCES "src/*.cpp" "src/*.h")
add_executable(NobleEngine ${CPP_SOURCES} "res/resource.rc")

# Include source headers
target_include_directories(NobleEngine PRIVATE ${CMAKE_SOURCE_DIR}/src)
# Ensure C++ 23 features
target_compile_features(NobleEngine PRIVATE cxx_std_23)

##########################################################
#                       VULKAN SDK                       #
##########################################################

# Vulkan SDK setup
find_package(Vulkan REQUIRED)
target_include_directories(NobleEngine PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(NobleEngine PRIVATE ${Vulkan_LIBRARIES})

#Vulkan debug utils
option(ENABLE_VULKAN_DEBUG_UTILS "Enable Vulkan debug utils extension" ON)

if(ENABLE_VULKAN_DEBUG_UTILS)
    add_compile_definitions(VULKAN_DEBUG_UTILS)
endif()

##########################################################
#                  SHADERS COMPILATION                   #
##########################################################

# Slang setup
set(SLANG_ROOT ${CMAKE_SOURCE_DIR}/lib/slang)
set(SLANGC_EXECUTABLE "${SLANG_ROOT}/bin/slangc.exe")
set(SLANG_INCLUDE_DIR "${SLANG_ROOT}/include")
set(SLANG_LIB_DIR "${SLANG_ROOT}/lib")

include_directories(${SLANG_INCLUDE_DIR})
link_directories(${SLANG_LIB_DIR})

# Slang compilation
function(add_slang_shader_target TARGET)
    cmake_parse_arguments(SHADERS "" "" "SOURCES;ENTRIES" ${ARGN})

    set(SHADERS_DIR ${CMAKE_SOURCE_DIR}/shaders/spv)
    file(MAKE_DIRECTORY ${SHADERS_DIR})

    set(SPV_OUTPUTS "")

    foreach(SOURCE ${SHADERS_SOURCES})
        cmake_path(GET SOURCE STEM SOURCE_NAME)

        foreach(ENTRY_PAIR ${SHADERS_ENTRIES})
            string(REPLACE "=" ";" ENTRY_SPLIT "${ENTRY_PAIR}")
            list(GET ENTRY_SPLIT 0 ENTRY_POINT)
            list(GET ENTRY_SPLIT 1 STAGE_NAME )

            string(SUBSTRING "${STAGE_NAME}" 0 4 STAGE_NAME_SHORT)

            set(SPV_FILE ${SHADERS_DIR}/${SOURCE_NAME}.${STAGE_NAME_SHORT}.spv)
            list(APPEND SPV_OUTPUTS ${SPV_FILE})

            # Run Slangc command
            add_custom_command(
                OUTPUT ${SPV_FILE}
                COMMAND ${SLANGC_EXECUTABLE} ${SOURCE}
                    -target spirv
                    -entry ${ENTRY_POINT}
                    -stage ${STAGE_NAME}
                    -profile spirv_1_4
                    -emit-spirv-directly
                    -fvk-use-entrypoint-name
                    -o ${SPV_FILE}
                DEPENDS ${SOURCE}
                COMMENT "Building SPIR-V ${SPV_FILE}"
                VERBATIM
            )
        endforeach()
    endforeach()

    add_custom_target(${TARGET} DEPENDS ${SPV_OUTPUTS})
endfunction()

file(GLOB_RECURSE SHADERS_SOURCES "shaders/src/*.slang")
add_slang_shader_target(ShadersModule
    SOURCES ${SHADERS_SOURCES}
    ENTRIES "fragMain=fragment" "vertMain=vertex"
)

add_dependencies(NobleEngine ShadersModule)
