#pragma once

#include "nodes/VulkanRenderPass.h"

struct VulkanRenderGraphBuilderContext;

class VulkanRenderPassFactory {
public:
    void registerPassTypes();

    [[nodiscard]] std::unique_ptr<VulkanRenderPass> createPass(
        const std::string&                     path,
        VulkanRenderPassType                   type,
        const VulkanRenderGraphBuilderContext& context,
        std::string&                           errorMessage
    ) const;

private:
    template <typename PassType, typename PassCreateContext>
    [[nodiscard]] static std::unique_ptr<VulkanRenderPass> createPassFactory(
        const std::string& path, const VulkanRenderGraphBuilderContext& context, std::string& errorMessage
    ) {
        auto pass = std::make_unique<PassType>();

        if (!pass->create(path, PassCreateContext::build(context), errorMessage)) {
            return nullptr;
        }

        return pass;
    }

    using PassFactoryFunction = std::function<std::unique_ptr<VulkanRenderPass>(
        const std::string&, const VulkanRenderGraphBuilderContext&, std::string&
    )>;

    std::unordered_map<VulkanRenderPassType, PassFactoryFunction> _factories{};
};
