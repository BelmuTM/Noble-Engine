#pragma once

#include <cstdint>
#include <string>

struct VulkanDispatchCall {
    std::string name = "Undefined_Dispatch_Call";

    std::uint32_t groupCountX = 1;
    std::uint32_t groupCountY = 1;
    std::uint32_t groupCountZ = 1;
};
