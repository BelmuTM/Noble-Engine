#pragma once

#include "core/debug/ErrorHandling.h"

#include "nodes/VulkanRenderPass.h"

struct VulkanRenderGraphBuilderContext;

class VulkanRenderPassFactory {
public:
    void registerPassTypes();

    [[nodiscard]] Expected<std::unique_ptr<VulkanRenderPass>> createPass(
        const VulkanRenderPassDescriptor&      descriptor,
        const VulkanRenderGraphBuilderContext& context
    ) const;

private:
    template<typename PassType, typename PassCreateContext>
    [[nodiscard]] static Expected<std::unique_ptr<VulkanRenderPass>> createPassFactory(
        const VulkanRenderPassDescriptor&      descriptor,
        const VulkanRenderGraphBuilderContext& context
    ) {
        auto pass = std::make_unique<PassType>(descriptor);

        TRY(pass->create(PassCreateContext::build(context)));

        return Expected<std::unique_ptr<VulkanRenderPass>>(std::move(pass));
    }

    using PassFactoryFunction = std::function<
        Expected<std::unique_ptr<VulkanRenderPass>>(
            const VulkanRenderPassDescriptor&,
            const VulkanRenderGraphBuilderContext&
        )
    >;

    std::unordered_map<VulkanRenderPassType, PassFactoryFunction> _factories{};
};
