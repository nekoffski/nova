#pragma once

#include "starlight/core/math/Core.hh"

#include "starlight/renderer/gpu/RenderPassBackend.hh"

#include "Vulkan.hh"
#include "fwd.hh"

namespace sl::vk {

class VulkanRenderPassBackend : public RenderPassBackend {
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
    explicit VulkanRenderPassBackend(
      VulkanDevice& device, const Properties& properties, bool hasPreviousPass,
      bool hasNextPass
    );

    ~VulkanRenderPassBackend() override;

    void begin(CommandBuffer& commandBuffer, u32 imageIndex) override;
    void end(CommandBuffer& commandBuffer) override;

    VkRenderPass getHandle();

protected:
    void generateRenderTargets();

    std::vector<VkClearValue> createClearValues(ClearFlags flags) const;

    VulkanDevice& m_device;
    VkRenderPass m_handle;

    Properties m_props;

    bool m_hasColorAttachment;
    bool m_hasDepthAttachment;

    std::vector<LocalPtr<Framebuffer>> m_framebuffers;
};

class VulkanImguiRenderPassBackend : public VulkanRenderPassBackend {
public:
    explicit VulkanImguiRenderPassBackend(
      VulkanDevice& device, const Properties& properties, bool hasPreviousPass,
      bool hasNextPass, const std::string& fontsPath
    );

    ~VulkanImguiRenderPassBackend() override;

    void begin(CommandBuffer& commandBuffer, u32 imageIndex) override;
    void end(CommandBuffer& commandBuffer) override;

private:
    void loadFonts(const std::string& fontsPath);
    void setStyles();
};

}  // namespace sl::vk
