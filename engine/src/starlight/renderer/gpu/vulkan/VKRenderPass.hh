#pragma once

#include "starlight/core/math/Core.hh"

#include "starlight/renderer/gpu/RenderPass.hh"

#include "VKPhysicalDevice.hh"
#include "VKContext.hh"

#include "Vulkan.hh"
#include "fwd.hh"

namespace sl::vk {

class VKRenderPass : public RenderPass {
public:
    enum class State : unsigned char {
        ready,
        recording,
        inRenderPass,
        recordingEnded,
        submitted,
        notAllocated
    };

    explicit VKRenderPass(
      VKContext& context, VKLogicalDevice& device, const VKSwapchain& swapchain,
      const Properties& properties, ChainFlags chainFlags
    );

    ~VKRenderPass();

    void begin(CommandBuffer& commandBuffer, u8 attachmentIndex) override;
    void end(CommandBuffer& commandBuffer) override;

    void regenerateRenderTargets(const std::vector<RenderTarget>&) override;

    VkRenderPass getHandle();

private:
    void generateRenderTargets(const std::vector<RenderTarget>&);

    std::vector<VkClearValue> createClearValues(ClearFlags flags) const;

    VkRenderPassBeginInfo createRenderPassBeginInfo(
      const std::vector<VkClearValue>& clearValues, VkFramebuffer framebuffer
    ) const;

    VKContext& m_context;
    VKLogicalDevice& m_device;

    VkRenderPass m_handle;
    float m_depth      = 1.0f;
    uint32_t m_stencil = 0;

    State m_state;

    // TODO: use local mem pool or static array or something
    std::vector<OwningPtr<VKFramebuffer>> m_framebuffers;
};

}  // namespace sl::vk
