#pragma once

#include "core/debug/ErrorHandling.h"

#include "nodes/VulkanRenderPass.h"

struct VulkanRenderGraphBuilderContext;

class VulkanRenderPassFactory {
public:
    void registerPassTypes();

    [[nodiscard]] Expected<std::unique_ptr<VulkanRenderPass>> createPass(
        const std::string&                     path,
        VulkanRenderPassType                   type,
        const VulkanRenderGraphBuilderContext& context
    ) const;

private:
    template<typename PassType, typename PassCreateContext>
    [[nodiscard]] static Expected<std::unique_ptr<VulkanRenderPass>> createPassFactory(
        const std::string& path, const VulkanRenderGraphBuilderContext& context
    ) {
        auto pass = std::make_unique<PassType>();

        TRY(pass->create(path, PassCreateContext::build(context)));

        return Expected<std::unique_ptr<VulkanRenderPass>>(std::move(pass));
    }

    using PassFactoryFunction = std::function<
        Expected<std::unique_ptr<VulkanRenderPass>>(
            const std::string&, const VulkanRenderGraphBuilderContext&
        )
    >;

    std::unordered_map<VulkanRenderPassType, PassFactoryFunction> _factories{};
};
