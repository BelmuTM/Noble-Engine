#include "VulkanRenderGraph.h"

#include "graphics/vulkan/resources/images/VulkanImageLayoutTransitions.h"

#include "core/debug/Logger.h"

bool VulkanRenderGraph::create(
    const VulkanMeshManager& meshManager,
    VulkanFrameResources&    frame,
    VulkanRenderResources&   resources,
    const vk::QueryPool      queryPool,
    std::string&             errorMessage
) noexcept {
    _meshManager = &meshManager;
    _frame       = &frame;
    _resources   = &resources;
    _queryPool   = queryPool;

    return true;
}

void VulkanRenderGraph::destroy() noexcept {
    _meshManager = nullptr;
    _frame       = nullptr;
    _resources   = nullptr;
}

void VulkanRenderGraph::attachSwapchainOutput(const VulkanSwapchain& swapchain) const {
    VulkanRenderPassResource swapchainOutput{};
    swapchainOutput
        .setType(SwapchainOutput)
        .setLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setFormat(swapchain.getFormat())
        .setImageViewResolver([this] { return _frame->getFrameContext().swapchainImageView; });

    VulkanRenderPassAttachment swapchainAttachment{};
    swapchainAttachment
        .setResource(swapchainOutput)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(defaultClearColor);

    VulkanRenderPass* lastPass = _passes.back().get();
    lastPass->addColorAttachmentAtIndex(0, swapchainAttachment);
}

void VulkanRenderGraph::execute(const vk::CommandBuffer commandBuffer) const {
    std::string errorMessage;
    ScopeGuard guard{[&errorMessage] { Logger::error(errorMessage); }};

    for (const auto& pass : _passes) {
        if (!executePass(commandBuffer, pass.get(), errorMessage)) return;
    }

    guard.release();
}

bool VulkanRenderGraph::executePass(
    const vk::CommandBuffer commandBuffer, const VulkanRenderPass* pass, std::string& errorMessage
) const {
    const vk::Buffer& vertexBuffer = _meshManager->getVertexBuffer();
    const vk::Buffer& indexBuffer  = _meshManager->getIndexBuffer();

    const VulkanFrameContext&  frameContext  = _frame->getFrameContext();
    const VulkanShaderProgram* shaderProgram = pass->getPipelineDescriptor().shaderProgram;

    if (pass->getBindPoint() == vk::PipelineBindPoint::eGraphics) {
        /*---------------------------------------*/
        /*     Rendering Info & Attachments      */
        /*---------------------------------------*/

        // Attach color buffers

        std::vector<vk::RenderingAttachmentInfo> colorAttachmentsInfo{};

        for (const auto& colorAttachment : pass->getColorAttachments()) {
            auto& colorResource = colorAttachment->resource;

            if (colorResource.image) {
                TRY(VulkanImageLayoutTransitions::transitionImageLayout(
                    commandBuffer,
                    *colorResource.image,
                    colorResource.format,
                    colorResource.layout,
                    vk::ImageLayout::eColorAttachmentOptimal,
                    1,
                    errorMessage
                ));
            }

            colorResource.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

            colorAttachmentsInfo.push_back(
                vk::RenderingAttachmentInfo{}
                    .setImageView(colorResource.resolveImageView())
                    .setImageLayout(colorResource.layout)
                    .setLoadOp(colorAttachment->loadOp)
                    .setStoreOp(colorAttachment->storeOp)
                    .setClearValue(colorAttachment->clearValue)
            );
        }

        // Rendering info

        vk::RenderingInfo renderingInfo{};
        renderingInfo
            .setRenderArea({{0, 0}, frameContext.extent})
            .setLayerCount(1)
            .setColorAttachments(colorAttachmentsInfo);

        // Attach depth buffer

        const VulkanRenderPassAttachment* depthAttachment = pass->getDepthAttachment();
        auto& depthResource = _resources->getDepthBufferAttachment()->resource;

        vk::ImageLayout targetDepthLayout = depthResource.layout;

        if (depthAttachment) {
            // Pass writes depth
            targetDepthLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        } else if (pass->readsDepthBuffer()) {
            // Pass reads depth
            targetDepthLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        }

        if (depthResource.layout != targetDepthLayout) {
            TRY(VulkanImageLayoutTransitions::transitionImageLayout(
                commandBuffer,
                *depthResource.image,
                depthResource.format,
                depthResource.layout,
                targetDepthLayout,
                1,
                errorMessage
            ));
            depthResource.setLayout(targetDepthLayout);
        }

        if (depthAttachment) {
            vk::RenderingAttachmentInfo depthAttachmentInfo{};
            depthAttachmentInfo
                .setImageView(depthResource.resolveImageView())
                .setImageLayout(depthResource.layout)
                .setLoadOp(depthAttachment->loadOp)
                .setStoreOp(depthAttachment->storeOp)
                .setClearValue(depthAttachment->clearValue);

            renderingInfo.setPDepthAttachment(&depthAttachmentInfo);
        }

        commandBuffer.resetQueryPool(_queryPool, 0, 1);

        commandBuffer.beginRendering(renderingInfo);

        commandBuffer.beginQuery(_queryPool, 0, {});

        commandBuffer.bindPipeline(pass->getBindPoint(), *pass->getPipeline());

        for (const auto& drawCall : pass->getDrawCalls()) {
            const auto& draw = *drawCall;

            const vk::PipelineLayout pipelineLayout = pass->getPipeline()->getLayout();

            /*---------------------------------------*/
            /*            Bind Resources             */
            /*---------------------------------------*/

            // Descriptors

            std::vector<vk::DescriptorSet> descriptorSets{};
            descriptorSets.push_back(_frame->getDescriptors()->getSets().at(frameContext.frameIndex));

            if (draw.descriptorResolver) {
                if (const auto objectSet = draw.descriptorResolver(frameContext); !objectSet.empty()) {
                    descriptorSets.push_back(objectSet[0]);
                }
            }

            for (const auto& descriptorSet : _resources->buildDescriptorSets(frameContext.frameIndex)) {
                descriptorSets.push_back(descriptorSet);
            }

            if (!descriptorSets.empty()) {
                commandBuffer.bindDescriptorSets(
                    pass->getBindPoint(), pipelineLayout, 0, descriptorSets, nullptr
                );
            }

            // Push constants

            if (auto* drawPushConstant = dynamic_cast<const VulkanDrawCallWithPushConstants*>(&draw)) {
                drawPushConstant->pushConstants(commandBuffer, pipelineLayout, shaderProgram);
            }

            /*---------------------------------------*/
            /*              Draw Meshes              */
            /*---------------------------------------*/

            commandBuffer.setViewport(0, draw.resolveViewport(frameContext));
            commandBuffer.setScissor(0, draw.resolveScissor(frameContext));

            if (!draw.mesh->isBufferless()) {
                const vk::DeviceSize vertexOffset = draw.mesh->getVertexOffset();
                const vk::DeviceSize indexOffset  = draw.mesh->getIndexOffset();

                commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer, &vertexOffset);
                commandBuffer.bindIndexBuffer(indexBuffer, indexOffset, vk::IndexType::eUint32);
                commandBuffer.drawIndexed(draw.mesh->getIndices().size(), 1, 0, 0, 0);
            } else {
                commandBuffer.draw(draw.mesh->getVertices().size(), 1, 0, 0);
            }
        }

        commandBuffer.endQuery(_queryPool, 0);

        commandBuffer.endRendering();

        for (const auto& [resource, targetLayout] : pass->getTransitions()) {
            TRY(VulkanImageLayoutTransitions::transitionImageLayout(
                commandBuffer,
                *resource->image,
                resource->format,
                resource->layout,
                targetLayout,
                1,
                errorMessage
            ));

            resource->setLayout(targetLayout);
        }
    }

    return true;
}
