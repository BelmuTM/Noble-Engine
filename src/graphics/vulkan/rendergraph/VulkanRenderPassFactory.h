#pragma once

#include "core/debug/ErrorHandling.h"

#include "nodes/VulkanGraphicsPass.h"

struct VulkanRenderGraphBuilderContext;

class VulkanRenderPassFactory {
public:
    void registerPassTypes();

    [[nodiscard]] Expected<void> createPass(
        VulkanGraphicsPass* pass,
        const VulkanRenderGraphBuilderContext& context
    ) const;

private:
    template<typename PassType, typename PassCreateContext>
    [[nodiscard]] static Expected<void> createPassFactory(
        VulkanGraphicsPass* pass, const VulkanRenderGraphBuilderContext& context
    ) {
        TRY(static_cast<PassType*>(pass)->create(PassCreateContext::build(context)));

        return {};
    }

    using PassFactoryFunction = std::function<
        Expected<void>(VulkanGraphicsPass*, const VulkanRenderGraphBuilderContext&)
    >;

    std::unordered_map<VulkanGraphicsPassType, PassFactoryFunction> _factories{};
};
