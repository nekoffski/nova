#include "VKRenderPass.hh"

#include "VKCommandBuffer.hh"
#include "VKSwapchain.hh"
#include "VKContext.hh"
#include "VKFramebuffer.hh"

namespace sl::vk {

class VKRenderPassCreateInfo {
public:
    explicit VKRenderPassCreateInfo(
      VkFormat depthFormat, VkSurfaceFormatKHR surfaceFormat,
      VKRenderPass::Properties props, RenderPass::ChainFlags chainFlags
    );

    VkRenderPassCreateInfo handle;

private:
    void createColorAttachment(
      VkSurfaceFormatKHR format, VKRenderPass::Properties props,
      RenderPass::ChainFlags chainFlags
    );
    void createDepthAttachment(VkFormat depthFormat, VKRenderPass::Properties props);
    void createSubpass();
    void createRenderPassDependencies();
    void createRenderPassCreateInfo();

    VkSubpassDescription m_subpass;

    std::vector<VkAttachmentDescription> m_attachmentDescriptions;

    VkAttachmentDescription m_colorAttachment;
    VkAttachmentReference m_colorAttachmentReference;

    VkAttachmentDescription m_depthAttachment;
    VkAttachmentReference m_depthAttachmentReference;

    VkSubpassDependency m_dependency;
};

VKRenderPass::VKRenderPass(
  VKContext& context, VKLogicalDevice& device, const VKSwapchain& swapchain,
  const Properties& properties, ChainFlags chainFlags
) : RenderPass(properties), m_context(context), m_device(device) {
    LOG_TRACE("Creating VKRenderPass instance");

    VKRenderPassCreateInfo createInfo(
      m_device.getDepthFormat(), swapchain.getSurfaceFormat(), properties, chainFlags
    );

    VK_ASSERT(vkCreateRenderPass(
      m_device.getHandle(), &createInfo.handle, m_context.getAllocator(), &m_handle
    ));
    LOG_TRACE("Vulkan render pass handle created");

    if (properties.renderTargets.size() == 0)
        LOG_WARN("Render pass with no render targets created");
    generateRenderTargets();
    LOG_TRACE("VKRenderPass instance created");
}

VKRenderPass::~VKRenderPass() {
    if (m_handle) {
        m_device.waitIdle();
        vkDestroyRenderPass(
          m_device.getHandle(), m_handle, m_context.getAllocator()
        );
    }
}

VkRenderPass VKRenderPass::getHandle() { return m_handle; }

std::vector<VkClearValue> VKRenderPass::createClearValues(ClearFlags flags) const {
    std::vector<VkClearValue> clearValues;
    clearValues.resize(2);

    if (isFlagEnabled(flags, ClearFlags::color)) {
        VkClearValue clearValue;

        clearValue.color.float32[0] = m_props.clearColor.r;
        clearValue.color.float32[1] = m_props.clearColor.g;
        clearValue.color.float32[2] = m_props.clearColor.b;
        clearValue.color.float32[3] = m_props.clearColor.a;

        clearValues[0] = clearValue;
    }

    if (isFlagEnabled(flags, ClearFlags::depth)) {
        VkClearValue clearValue;
        clearValue.depthStencil.depth = m_depth;

        if (isFlagEnabled(flags, ClearFlags::stencil))
            clearValue.depthStencil.stencil = m_stencil;

        clearValues[1] = clearValue;
    }

    return clearValues;
}

VkRenderPassBeginInfo VKRenderPass::createRenderPassBeginInfo(
  const std::vector<VkClearValue>& clearValues, VkFramebuffer framebuffer
) const {
    VkRenderPassBeginInfo beginInfo;
    clearMemory(&beginInfo);
    beginInfo.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.renderPass               = m_handle;
    beginInfo.framebuffer              = framebuffer;
    beginInfo.renderArea.offset.x      = m_props.rect.offset.x;
    beginInfo.renderArea.offset.y      = m_props.rect.offset.y;
    beginInfo.renderArea.extent.width  = m_props.rect.size.w;
    beginInfo.renderArea.extent.height = m_props.rect.size.h;

    beginInfo.clearValueCount = clearValues.size();
    beginInfo.pClearValues =
      beginInfo.clearValueCount > 0 ? clearValues.data() : nullptr;

    return beginInfo;
}

void VKRenderPass::begin(CommandBuffer& commandBuffer, u8 attachmentIndex) {
    auto clearValues = createClearValues(m_props.clearFlags);

    auto beginInfo = createRenderPassBeginInfo(
      clearValues, m_framebuffers[attachmentIndex]->getHandle()
    );

    auto vkBuffer = static_cast<VKCommandBuffer*>(&commandBuffer);

    vkCmdBeginRenderPass(
      vkBuffer->getHandle(), &beginInfo, VK_SUBPASS_CONTENTS_INLINE
    );
    vkBuffer->setState(VKCommandBuffer::State::inRenderPass);
}

void VKRenderPass::end(CommandBuffer& commandBuffer) {
    auto vkBuffer = static_cast<VKCommandBuffer*>(&commandBuffer);
    vkCmdEndRenderPass(vkBuffer->getHandle());
    vkBuffer->setState(VKCommandBuffer::State::recording);
}

void VKRenderPass::regenerateRenderTargets(const Vec2<u32>& viewportSize) {
    LOG_TRACE("Regenerating render targets for render pass");
    for (auto& renderTarget : m_props.renderTargets)
        renderTarget.size = viewportSize;
    m_framebuffers.clear();
    generateRenderTargets();
}

static void addAttachment(
  std::vector<VkImageView>& attachmentViews, Texture* attachment
) {
    attachmentViews.push_back(
      static_cast<VKTexture*>(attachment)->getImage()->getView()
    );
}

void VKRenderPass::generateRenderTargets() {
    auto& renderTargets = m_props.renderTargets;
    LOG_TRACE("Generating render targets for render pass");
    for (const auto& renderTarget : renderTargets) {
        std::vector<VkImageView> attachmentViews;

        if (renderTarget.colorAttachment != nullptr)
            addAttachment(attachmentViews, renderTarget.colorAttachment);

        if (renderTarget.depthAttachment != nullptr)
            addAttachment(attachmentViews, renderTarget.depthAttachment);

        LOG_TRACE(
          "Creating framebuffer for render target, attachment count: {}",
          attachmentViews.size()
        );
        m_framebuffers.push_back(createOwningPtr<VKFramebuffer>(
          m_context, m_device, m_handle, renderTarget.size.x, renderTarget.size.y,
          attachmentViews
        ));
    }
}

VKRenderPassCreateInfo::VKRenderPassCreateInfo(
  VkFormat depthFormat, VkSurfaceFormatKHR surfaceFormat,
  VKRenderPass::Properties props, RenderPass::ChainFlags chainFlags
) {
    m_attachmentDescriptions.reserve(2);
    bool hasColorAttachment = false;
    bool hasDepthAttachment = false;

    for (auto& target : props.renderTargets) {
        if (target.colorAttachment != nullptr) hasColorAttachment = true;
        if (target.depthAttachment != nullptr) hasDepthAttachment = true;
    }

    if (hasColorAttachment) {
        createColorAttachment(surfaceFormat, props, chainFlags);
        m_colorAttachmentReference.attachment = m_attachmentDescriptions.size() - 1;
        m_colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        m_subpass.colorAttachmentCount    = 1;
        m_subpass.pColorAttachments       = &m_colorAttachmentReference;
    } else {
        m_subpass.colorAttachmentCount = 0;
    }

    if (hasDepthAttachment) {
        createDepthAttachment(depthFormat, props);
        m_depthAttachmentReference.attachment = m_attachmentDescriptions.size() - 1;
        m_depthAttachmentReference.layout     = VK_IMAGE_LAYOUT_GENERAL;
        m_subpass.pDepthStencilAttachment     = &m_depthAttachmentReference;
    } else {
        m_subpass.pDepthStencilAttachment = nullptr;
    }

    createSubpass();
    createRenderPassDependencies();
    createRenderPassCreateInfo();
}

void VKRenderPassCreateInfo::createColorAttachment(
  VkSurfaceFormatKHR surfaceFormat, VKRenderPass::Properties props,
  RenderPass::ChainFlags chainFlags
) {
    m_colorAttachment.format  = surfaceFormat.format;  // TODO: configurable
    m_colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    m_colorAttachment.loadOp =
      isFlagEnabled(props.clearFlags, RenderPass::ClearFlags::color)
        ? VK_ATTACHMENT_LOAD_OP_CLEAR
        : VK_ATTACHMENT_LOAD_OP_LOAD;
    m_colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    m_colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    m_colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // Do not expect any particular layout before render pass starts.
    m_colorAttachment.initialLayout =
      isFlagEnabled(chainFlags, RenderPass::ChainFlags::hasPrevious)
        ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        : VK_IMAGE_LAYOUT_UNDEFINED;
    m_colorAttachment.finalLayout =
      isFlagEnabled(chainFlags, RenderPass::ChainFlags::hasNext)
        ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    m_colorAttachment.flags = 0;

    m_attachmentDescriptions.push_back(m_colorAttachment);
}

void VKRenderPassCreateInfo::createDepthAttachment(
  VkFormat depthFormat, VKRenderPass::Properties props
) {
    bool clearDepth = isFlagEnabled(props.clearFlags, RenderPass::ClearFlags::depth);

    m_depthAttachment.format  = depthFormat;
    m_depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    m_depthAttachment.loadOp =
      clearDepth ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    m_depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    m_depthAttachment.initialLayout =
      clearDepth ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_GENERAL;
    m_depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_GENERAL;
    m_depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    m_depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    m_depthAttachment.flags          = 0;

    m_attachmentDescriptions.push_back(m_depthAttachment);
}

void VKRenderPassCreateInfo::createSubpass() {
    m_subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    m_subpass.inputAttachmentCount    = 0;
    m_subpass.pInputAttachments       = 0;
    m_subpass.pResolveAttachments     = 0;
    m_subpass.preserveAttachmentCount = 0;
    m_subpass.pPreserveAttachments    = 0;
    m_subpass.flags                   = 0;
}

void VKRenderPassCreateInfo::createRenderPassDependencies() {
    m_dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    m_dependency.dstSubpass    = 0;
    m_dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    m_dependency.srcAccessMask = 0;
    m_dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    m_dependency.dstAccessMask =
      VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    m_dependency.dependencyFlags = 0;
}

void VKRenderPassCreateInfo::createRenderPassCreateInfo() {
    handle.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    handle.attachmentCount = m_attachmentDescriptions.size();
    handle.pAttachments    = m_attachmentDescriptions.data();
    handle.subpassCount    = 1;
    handle.pSubpasses      = &m_subpass;
    handle.dependencyCount = 1;
    handle.pDependencies   = &m_dependency;
    handle.pNext           = 0;
    handle.flags           = 0;
}

}  // namespace sl::vk
