#[[
    Vulkan-Headers
        * Upstream: https://github.com/KhronosGroup/Vulkan-Headers
        * License: Apache 2.0
]]

function (setup_vulkan TARGET)

    add_subdirectory(external/vulkan-headers)

    target_include_directories(${TARGET} SYSTEM PRIVATE external/vulkan-headers/include)

    # Determine if the Vulkan SDK is installed on the system
    message(CHECK_START "Check for Vulkan SDK")

    find_package(Vulkan QUIET)

    if (Vulkan_FOUND)

        message(CHECK_PASS "done")

        target_compile_definitions(${TARGET} PRIVATE VULKAN_SDK)

        target_link_libraries(NobleEngine PRIVATE Vulkan::Vulkan)

    else()

        message(CHECK_FAIL "failed (validation layers will be disabled!)")

        target_link_libraries(NobleEngine PRIVATE vulkan)

    endif()

    target_compile_definitions(${TARGET} PRIVATE
        VULKAN_HPP_NO_EXCEPTIONS
        # VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    )

    # Vulkan debug utils
    target_compile_definitions(${TARGET} PRIVATE $<$<CONFIG:Debug>:VULKAN_DEBUG_UTILS>)

endfunction()
