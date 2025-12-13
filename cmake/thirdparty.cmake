#[[
    Third-party libraries

    Ordered alphabetically

    glfw
        * Upstream: https://github.com/glfw/glfw
        * License: Zlib

    glm
        * Upstream: https://github.com/g-truc/glm
        * License: MIT

    json
        * Upstream: https://github.com/nlohmann/json
        * License: MIT

    SPIRV-Headers
        * Upstream: https://github.com/KhronosGroup/SPIRV-Headers

    SPIRV-Reflect
        * Upstream: https://github.com/KhronosGroup/SPIRV-Reflect
        * License: Apache 2.0

    STB
        * Upstream: https://github.com/nothings/stb
        * License: MIT

    tinygltf
        * Upstream: https://github.com/syoyo/tinygltf
        * License: MIT

    tinyobjloader
        * Upstream: https://github.com/tinyobjloader/tinyobjloader
        * License: MIT

    VulkanMemoryAllocator
        * Upstream: https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
        * License: MIT
]]

function (setup_thirdparty TARGET)

    # glfw
    add_subdirectory(external/glfw)

    # glm
    add_subdirectory(external/glm)

    target_compile_definitions(
        glm
        INTERFACE
        GLM_FORCE_CXX20
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_ENABLE_EXPERIMENTAL
    )

    # SPIRV
    set(SPIRV-HEADERS_Libraries "${CMAKE_SOURCE_DIR}/external/spirv-headers/include")

    add_library(spirv-reflect STATIC external/spirv-reflect/spirv_reflect.cpp)

    target_include_directories(spirv-reflect SYSTEM PRIVATE ${SPIRV-HEADERS_Libraries})

    target_compile_definitions(spirv-reflect PUBLIC SPIRV_REFLECT_USE_SYSTEM_SPIRV_H)

    # tinygltf
    target_compile_definitions(NobleEngine PRIVATE
        TINYGLTF_NO_EXTERNAL_IMAGE
        TINYGLTF_NO_STB_IMAGE
        TINYGLTF_NO_STB_IMAGE_WRITE
    )

    # Link compiled libraries to executable
    target_link_libraries(
        NobleEngine
        PRIVATE
        glfw
        glm
        spirv-reflect
    )

    # Include directories
    target_include_directories(
        NobleEngine
        SYSTEM
        PRIVATE
        ${CMAKE_SOURCE_DIR}/external
        ${CMAKE_SOURCE_DIR}/external/json
        ${CMAKE_SOURCE_DIR}/external/stb
        ${SPIRV-HEADERS_Libraries}
    )

endfunction()
