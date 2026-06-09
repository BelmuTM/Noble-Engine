#pragma once

#include "core/debug/ErrorHandling.h"

#include "nodes/VulkanRenderPass.h"

struct VulkanRenderGraphBuilderContext;

class VulkanRenderPassFactory {
public:
    void registerPassTypes();

    [[nodiscard]] Expected<void> createPass(
        VulkanRenderPass* pass,
        const VulkanRenderGraphBuilderContext& context
    ) const;

private:
    template<typename PassType, typename PassCreateContext>
    [[nodiscard]] static Expected<void> createPassFactory(
        VulkanRenderPass* pass, const VulkanRenderGraphBuilderContext& context
    ) {
        TRY(static_cast<PassType*>(pass)->create(PassCreateContext::build(context)));

        return {};
    }

    using PassFactoryFunction = std::function<
        Expected<void>(VulkanRenderPass*, const VulkanRenderGraphBuilderContext&)
    >;

    std::unordered_map<VulkanRenderPassType, PassFactoryFunction> _factories{};
};
