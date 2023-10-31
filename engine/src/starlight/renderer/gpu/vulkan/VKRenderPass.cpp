#include "VKRenderPass.h"

#include "VKDevice.h"
#include "VKCommandBuffer.h"
#include "VKSwapchain.h"
#include "VKContext.h"

namespace sl::vk {

// TODO: refactor
struct RenderPassCreateInfo {
    explicit RenderPassCreateInfo(
      VkFormat depthFormat, VkSurfaceFormatKHR surfaceFormat,
      VKRenderPass::Properties props
    ) :
        props(props) {
        createColorAttachment(surfaceFormat);
        if (props.clearFlags & VKRenderPass::clearFlagDepthBuffer)
            createDepthAttachment(depthFormat);

        createSubpass();

        createRenderPassDependencies();
        createRenderPassCreateInfo();
    }

    //  TODO: ASSIGN STRUCTURE TYPES IF MISSING!
    void createSubpass() {
        subpass.pipelineBindPoint =
          VK_PIPELINE_BIND_POINT_GRAPHICS;  // Depth stencil data.

        subpass.pDepthStencilAttachment = nullptr;

        if (props.clearFlags & VKRenderPass::clearFlagDepthBuffer)
            subpass.pDepthStencilAttachment = &depthAttachmentReference;

        // Input from a shader
        subpass.inputAttachmentCount = 0;
        subpass.pInputAttachments    = 0;

        // Attachments used for multisampling colour attachments
        subpass.pResolveAttachments = 0;

        // Attachments not used in this subpass, but must be preserved for the next.
        subpass.preserveAttachmentCount = 0;
        subpass.pPreserveAttachments    = 0;
        subpass.flags                   = 0;
    }

    void createColorAttachment(VkSurfaceFormatKHR surfaceFormat) {
        colorAttachment.format  = surfaceFormat.format;  // TODO: configurable
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp =
          (props.clearFlags & VKRenderPass::clearFlagColorBuffer)
            ? VK_ATTACHMENT_LOAD_OP_CLEAR
            : VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        // Do not expect any particular layout before render pass starts.
        colorAttachment.initialLayout =
          props.hasPreviousPass
            ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            : VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout =
          props.hasNextPass
            ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        colorAttachment.flags = 0;

        attachmentDescriptions.push_back(colorAttachment);

        colorAttachmentReference.attachment =
          0;  // Attachment description array index
        colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments    = &colorAttachmentReference;
    }

    void createDepthAttachment(VkFormat depthFormat) {
        depthAttachment.format        = depthFormat;
        depthAttachment.samples       = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp       = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout =
          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.flags          = 0;

        attachmentDescriptions.push_back(depthAttachment);

        // Depth attachment reference
        depthAttachmentReference.attachment = 1;
        depthAttachmentReference.layout =
          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    void createRenderPassDependencies() {
        dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass    = 0;
        dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask =
          VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dependencyFlags = 0;
    }

    void createRenderPassCreateInfo() {
        handle.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        handle.attachmentCount = attachmentDescriptions.size();
        handle.pAttachments    = attachmentDescriptions.data();
        handle.subpassCount    = 1;
        handle.pSubpasses      = &subpass;
        handle.dependencyCount = 1;
        handle.pDependencies   = &dependency;
        handle.pNext           = 0;
        handle.flags           = 0;
    }

    VKRenderPass::Properties props;

    VkRenderPassCreateInfo handle;
    VkSubpassDescription subpass;

    std::vector<VkAttachmentDescription> attachmentDescriptions;

    VkAttachmentDescription colorAttachment;
    VkAttachmentReference colorAttachmentReference;

    VkAttachmentDescription depthAttachment;
    VkAttachmentReference depthAttachmentReference;

    VkSubpassDependency dependency;
};

VKRenderPass::VKRenderPass(
  const VKContext* context, const VKDevice* device, const VKSwapchain& swapchain,
  const Properties& properties
) :
    m_context(context),
    m_device(device), m_area(properties.area), m_color(properties.color),
    m_clearFlags(properties.clearFlags),
    m_hasPreviousPass(properties.hasPreviousPass),
    m_hasNextPass(properties.hasNextPass) {
    LOG_TRACE("Creating VKRenderPass instance");

    RenderPassCreateInfo createInfo(
      m_device->getDepthFormat(), swapchain.getSurfaceFormat(), properties
    );

    VK_ASSERT(vkCreateRenderPass(
      m_device->getLogicalDevice(), &createInfo.handle, m_context->getAllocator(),
      &m_handle
    ));
}

VKRenderPass::~VKRenderPass() {
    if (m_handle)
        vkDestroyRenderPass(
          m_device->getLogicalDevice(), m_handle, m_context->getAllocator()
        );
}

VkRenderPass VKRenderPass::getHandle() { return m_handle; }

std::vector<VkClearValue> VKRenderPass::createClearValues(
  VKRenderPass::ClearFlag flags
) const {
    std::vector<VkClearValue> clearValues;
    clearValues.reserve(2);

    if (flags & VKRenderPass::clearFlagColorBuffer) {
        VkClearValue clearValue;

        clearValue.color.float32[0] = m_color.r;
        clearValue.color.float32[1] = m_color.g;
        clearValue.color.float32[2] = m_color.b;
        clearValue.color.float32[3] = m_color.a;

        clearValues.push_back(clearValue);
    }

    if (flags & VKRenderPass::clearFlagDepthBuffer) {
        VkClearValue clearValue;
        clearValue.depthStencil.depth = m_depth;

        if (flags & VKRenderPass::clearFlagStencilBuffer)
            clearValue.depthStencil.stencil = m_stencil;

        clearValues.push_back(clearValue);
    }

    return clearValues;
}

VkRenderPassBeginInfo VKRenderPass::createRenderPassBeginInfo(
  const std::vector<VkClearValue>& clearValues, VkFramebuffer framebuffer
) const {
    VkRenderPassBeginInfo beginInfo   = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    beginInfo.renderPass              = m_handle;
    beginInfo.framebuffer             = framebuffer;
    beginInfo.renderArea.offset.x     = m_area.x;
    beginInfo.renderArea.offset.y     = m_area.y;
    beginInfo.renderArea.extent.width = m_area.z;
    beginInfo.renderArea.extent.height = m_area.w;

    beginInfo.clearValueCount = clearValues.size();
    beginInfo.pClearValues =
      beginInfo.clearValueCount > 0 ? clearValues.data() : nullptr;

    return beginInfo;
}

void VKRenderPass::begin(VKCommandBuffer& commandBuffer, VkFramebuffer framebuffer) {
    auto clearValues = createClearValues(m_clearFlags);
    auto beginInfo   = createRenderPassBeginInfo(clearValues, framebuffer);

    vkCmdBeginRenderPass(
      commandBuffer.getHandle(), &beginInfo, VK_SUBPASS_CONTENTS_INLINE
    );
    commandBuffer.setState(VKCommandBuffer::State::inRenderPass);
}

void VKRenderPass::end(VKCommandBuffer& commandBuffer) {
    vkCmdEndRenderPass(commandBuffer.getHandle());
    commandBuffer.setState(VKCommandBuffer::State::recording);
}

}  // namespace sl::vk