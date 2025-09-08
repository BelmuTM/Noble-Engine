cmake_minimum_required(VERSION 3.27)
project(BazarEngine LANGUAGES CXX)

# Set C++ standard and output directory
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Collect source files and add Windows resource file
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")
add_executable(BazarEngine ${SOURCES} "res/resource.rc")

# Vulkan SDK setup
find_package(Vulkan REQUIRED)
target_include_directories(BazarEngine PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(BazarEngine PRIVATE ${Vulkan_LIBRARIES})

#Vulkan debug utils
option(ENABLE_VULKAN_DEBUG_UTILS "Enable Vulkan debug utils extension" ON)

if(ENABLE_VULKAN_DEBUG_UTILS)
    add_compile_definitions(VULKAN_DEBUG_UTILS)
endif()

# Include source headers
target_include_directories(BazarEngine PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Ensure C++ 23 features
target_compile_features(BazarEngine PRIVATE cxx_std_23)
