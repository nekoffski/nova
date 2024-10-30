#include "VKFramebuffer.hh"

#include "VKContext.hh"

namespace sl::vk {

VkFramebufferCreateInfo createFramebufferCreateInfo(
  std::vector<VkImageView>& attachments, VkRenderPass renderPass, u32 width,
  u32 height
) {
    VkFramebufferCreateInfo createInfo;
    clearMemory(&createInfo);

    createInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.renderPass      = renderPass;
    createInfo.attachmentCount = attachments.size();
    createInfo.pAttachments    = attachments.data();
    createInfo.width           = width;
    createInfo.height          = height;
    createInfo.layers          = 1;

    return createInfo;
}

VkFramebuffer VKFramebuffer::getHandle() { return m_handle; }

VKFramebuffer::VKFramebuffer(
  VKContext& context, VKLogicalDevice& device, VkRenderPass renderPass, u32 width,
  u32 height, const std::vector<VkImageView>& attachments
) : m_context(context), m_device(device), m_attachments(attachments) {
    const auto createInfo =
      createFramebufferCreateInfo(m_attachments, renderPass, width, height);

    VK_ASSERT(vkCreateFramebuffer(
      m_device.getHandle(), &createInfo, m_context.getAllocator(), &m_handle
    ));
}

VKFramebuffer::~VKFramebuffer() {
    if (m_handle) {
        vkDestroyFramebuffer(
          m_device.getHandle(), m_handle, m_context.getAllocator()
        );
    }
}

}  // namespace sl::vk
