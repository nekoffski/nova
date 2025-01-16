#pragma once

#include "starlight/core/math/Core.hh"

#include "starlight/renderer/gpu/RenderPass.hh"

#include "Vulkan.hh"
#include "fwd.hh"

namespace sl::vk {

class VulkanRenderPass : public RenderPass::Impl {
    class Framebuffer : public NonMovable, public NonCopyable {
    public:
        explicit Framebuffer(
          VulkanDevice& device, VkRenderPass renderPass, const Vec2<u32>& size,
          const std::vector<VkImageView>& attachments
        );
        ~Framebuffer();

        VkFramebuffer handle;

    private:
        VulkanDevice& m_device;
        std::vector<VkImageView> m_attachments;
    };

public:
    explicit VulkanRenderPass(
      VulkanDevice& device, const RenderPass::Properties& properties,
      bool hasPreviousPass, bool hasNextPass
    );

    ~VulkanRenderPass();

    void begin(CommandBuffer& commandBuffer, u32 imageIndex) override;
    void end(CommandBuffer& commandBuffer) override;

    VkRenderPass getHandle();

private:
    void generateRenderTargets();

    std::vector<VkClearValue> createClearValues(RenderPass::ClearFlags flags) const;

    VulkanDevice& m_device;
    VkRenderPass m_handle;

    RenderPass::Properties m_props;

    bool m_hasColorAttachment;
    bool m_hasDepthAttachment;

    std::vector<LocalPtr<Framebuffer>> m_framebuffers;
};

}  // namespace sl::vk
