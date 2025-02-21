#include "VulkanRenderPassBackend.hh"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>
#include <ImGuizmo.h>

#include "VulkanCommandBuffer.hh"
#include "VulkanSwapchain.hh"

#include "VulkanDevice.hh"

namespace sl::vk {

/*
    Framebuffer
*/

VulkanRenderPassBackend::Framebuffer::Framebuffer(
  VulkanDevice& device, VkRenderPass renderPass, const Vec2<u32>& size,
  const std::vector<VkImageView>& attachments
) : handle(VK_NULL_HANDLE), m_device(device) {
    VkFramebufferCreateInfo createInfo;
    clearMemory(&createInfo);

    createInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.renderPass      = renderPass;
    createInfo.attachmentCount = attachments.size();
    createInfo.pAttachments    = attachments.data();
    createInfo.width           = size.w;
    createInfo.height          = size.h;
    createInfo.layers          = 1;

    log::expect(vkCreateFramebuffer(
      m_device.logical.handle, &createInfo, m_device.allocator, &handle
    ));
    log::trace("vkCreateFramebuffer: {}", static_cast<void*>(handle));
}

VulkanRenderPassBackend::Framebuffer::~Framebuffer() {
    if (handle) {
        log::trace("vkDestroyFramebuffer: {}", static_cast<void*>(handle));
        vkDestroyFramebuffer(m_device.logical.handle, handle, m_device.allocator);
    }
}

/*
    VulkanRenderPassBackend
*/

class VulkanRenderPassBackendCreateInfo {
public:
    explicit VulkanRenderPassBackendCreateInfo(
      VkFormat depthFormat, VkSurfaceFormatKHR surfaceFormat,
      RenderPassBackend::Properties props, bool hasPreviousPass, bool hasNextPass,
      bool hasColorAttachment, bool hasDepthAttachment
    );

    VkRenderPassCreateInfo handle;

private:
    void createColorAttachment(
      VkSurfaceFormatKHR format, RenderPassBackend::Properties props,
      bool hasPreviousPass, bool hasNextPass
    );
    void createDepthAttachment(
      VkFormat depthFormat, RenderPassBackend::Properties props
    );
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

VulkanRenderPassBackend::VulkanRenderPassBackend(
  VulkanDevice& device, const RenderPassBackend::Properties& properties,
  bool hasPreviousPass, bool hasNextPass
) :
    m_device(device), m_handle(VK_NULL_HANDLE), m_props(properties),
    m_hasColorAttachment(false), m_hasDepthAttachment(false) {
    log::trace("Creating VulkanRenderPassBackend instance");

    for (auto& target : properties.renderTargets) {
        if (target.colorAttachment != nullptr) m_hasColorAttachment = true;
        if (target.depthAttachment != nullptr) m_hasDepthAttachment = true;
    }

    VulkanRenderPassBackendCreateInfo createInfo(
      m_device.physical.info.depthFormat, m_device.physical.info.surfaceFormat,
      properties, hasPreviousPass, hasNextPass, m_hasColorAttachment,
      m_hasDepthAttachment
    );

    log::expect(vkCreateRenderPass(
      m_device.logical.handle, &createInfo.handle, m_device.allocator, &m_handle
    ));
    log::trace("vkCreateRenderPass: {}", static_cast<void*>(m_handle));

    if (properties.renderTargets.size() == 0)
        log::warn("Render pass with no render targets created");

    generateRenderTargets();
    log::trace("VulkanRenderPassBackend instance created");
}

VulkanRenderPassBackend::~VulkanRenderPassBackend() {
    if (m_handle) {
        m_device.waitIdle();
        log::trace("vkDestroyRenderPass: {}", static_cast<void*>(m_handle));
        vkDestroyRenderPass(m_device.logical.handle, m_handle, m_device.allocator);
    }
}

VkRenderPass VulkanRenderPassBackend::getHandle() { return m_handle; }

std::vector<VkClearValue> VulkanRenderPassBackend::createClearValues(ClearFlags flags
) const {
    std::vector<VkClearValue> clearValues;
    clearValues.reserve(2);

    if (m_hasColorAttachment) {
        VkClearValue clearValue;

        clearValue.color.float32[0] = m_props.clearColor.r;
        clearValue.color.float32[1] = m_props.clearColor.g;
        clearValue.color.float32[2] = m_props.clearColor.b;
        clearValue.color.float32[3] = m_props.clearColor.a;

        clearValues.push_back(clearValue);
    }

    if (m_hasDepthAttachment && isFlagEnabled(flags, ClearFlags::depth)) {
        VkClearValue clearValue;
        clearValue.depthStencil.depth = m_props.depth;

        if (isFlagEnabled(flags, ClearFlags::stencil))
            clearValue.depthStencil.stencil = m_props.stencil;

        clearValues.push_back(clearValue);
    }

    return clearValues;
}

void VulkanRenderPassBackend::begin(
  CommandBuffer& commandBuffer, u32 attachmentIndex
) {
    auto clearValues = createClearValues(m_props.clearFlags);

    VkRenderPassBeginInfo beginInfo;
    clearMemory(&beginInfo);
    beginInfo.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.renderPass               = m_handle;
    beginInfo.framebuffer              = m_framebuffers[attachmentIndex]->handle;
    beginInfo.renderArea.offset.x      = m_props.rect.offset.x;
    beginInfo.renderArea.offset.y      = m_props.rect.offset.y;
    beginInfo.renderArea.extent.width  = m_props.rect.size.w;
    beginInfo.renderArea.extent.height = m_props.rect.size.h;

    beginInfo.clearValueCount = clearValues.size();
    beginInfo.pClearValues    = clearValues.data();

    auto vkBuffer = static_cast<VulkanCommandBuffer*>(&commandBuffer);

    vkCmdBeginRenderPass(
      vkBuffer->getHandle(), &beginInfo, VK_SUBPASS_CONTENTS_INLINE
    );
}

void VulkanRenderPassBackend::end(CommandBuffer& commandBuffer) {
    auto vkBuffer = static_cast<VulkanCommandBuffer*>(&commandBuffer);
    vkCmdEndRenderPass(vkBuffer->getHandle());
}

static void addAttachment(
  std::vector<VkImageView>& attachmentViews, Texture* attachment
) {
    attachmentViews.push_back(static_cast<VulkanTexture*>(attachment)->getView());
}

void VulkanRenderPassBackend::generateRenderTargets() {
    auto& renderTargets = m_props.renderTargets;
    log::trace("Generating render targets for render pass");
    for (const auto& renderTarget : renderTargets) {
        std::vector<VkImageView> attachmentViews;

        if (renderTarget.colorAttachment != nullptr)
            addAttachment(attachmentViews, renderTarget.colorAttachment);

        if (renderTarget.depthAttachment != nullptr)
            addAttachment(attachmentViews, renderTarget.depthAttachment);

        log::trace(
          "Creating framebuffer for render target, attachment count: {}",
          attachmentViews.size()
        );

        m_framebuffers.emplace_back(
          m_device, m_handle, m_props.rect.offset + m_props.rect.size,
          attachmentViews
        );
    }
}

VulkanRenderPassBackendCreateInfo::VulkanRenderPassBackendCreateInfo(
  VkFormat depthFormat, VkSurfaceFormatKHR surfaceFormat,
  RenderPassBackend::Properties props, bool hasPreviousPass, bool hasNextPass,
  bool hasColorAttachment, bool hasDepthAttachment
) {
    m_attachmentDescriptions.reserve(2);

    if (hasColorAttachment) {
        createColorAttachment(surfaceFormat, props, hasPreviousPass, hasNextPass);
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

void VulkanRenderPassBackendCreateInfo::createColorAttachment(
  VkSurfaceFormatKHR surfaceFormat, RenderPassBackend::Properties props,
  bool hasPreviousPass, bool hasNextPass
) {
    m_colorAttachment.format  = surfaceFormat.format;  // TODO: configurable
    m_colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    m_colorAttachment.loadOp =
      isFlagEnabled(props.clearFlags, ClearFlags::color)
        ? VK_ATTACHMENT_LOAD_OP_CLEAR
        : VK_ATTACHMENT_LOAD_OP_LOAD;
    m_colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    m_colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    m_colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // Do not expect any particular layout before render pass starts.
    m_colorAttachment.initialLayout =
      hasPreviousPass
        ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        : VK_IMAGE_LAYOUT_UNDEFINED;
    m_colorAttachment.finalLayout =
      hasNextPass
        ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    m_colorAttachment.flags = 0;

    m_attachmentDescriptions.push_back(m_colorAttachment);
}

void VulkanRenderPassBackendCreateInfo::createDepthAttachment(
  VkFormat depthFormat, RenderPassBackend::Properties props
) {
    bool clearDepth = isFlagEnabled(props.clearFlags, ClearFlags::depth);

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

void VulkanRenderPassBackendCreateInfo::createSubpass() {
    m_subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    m_subpass.inputAttachmentCount    = 0;
    m_subpass.pInputAttachments       = 0;
    m_subpass.pResolveAttachments     = 0;
    m_subpass.preserveAttachmentCount = 0;
    m_subpass.pPreserveAttachments    = 0;
    m_subpass.flags                   = 0;
}

void VulkanRenderPassBackendCreateInfo::createRenderPassDependencies() {
    m_dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    m_dependency.dstSubpass    = 0;
    m_dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    m_dependency.srcAccessMask = 0;
    m_dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    m_dependency.dstAccessMask =
      VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    m_dependency.dependencyFlags = 0;
}

void VulkanRenderPassBackendCreateInfo::createRenderPassCreateInfo() {
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

/*
    VulkanImguiRenderPassBackend
*/

VulkanImguiRenderPassBackend::VulkanImguiRenderPassBackend(
  VulkanDevice& device, const Properties& properties, bool hasPreviousPass,
  bool hasNextPass, const std::string& fontsPath
) : VulkanRenderPassBackend(device, properties, hasPreviousPass, hasNextPass) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(
      static_cast<GLFWwindow*>(Window::get().getHandle()), true
    );

    ImGui_ImplVulkan_InitInfo initInfo;
    clearMemory(&initInfo);
    initInfo.Instance       = m_device.instance.handle;
    initInfo.PhysicalDevice = m_device.physical.handle;
    initInfo.Device         = m_device.logical.handle;
    initInfo.Queue          = m_device.getQueue(Queue::Type::graphics).getHandle();
    initInfo.DescriptorPool = m_device.uiDescriptorPool;
    initInfo.MinImageCount  = 3;
    initInfo.ImageCount     = 3;
    initInfo.MSAASamples    = VK_SAMPLE_COUNT_1_BIT;
    initInfo.RenderPass     = m_handle;

    ImGui_ImplVulkan_Init(&initInfo);

    loadFonts(fontsPath);
    setStyles();
}

VulkanImguiRenderPassBackend::~VulkanImguiRenderPassBackend() {
    m_device.waitIdle();

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void VulkanImguiRenderPassBackend::begin(
  CommandBuffer& commandBuffer, u32 imageIndex
) {
    VulkanRenderPassBackend::begin(commandBuffer, imageIndex);
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
}

void VulkanImguiRenderPassBackend::end(CommandBuffer& commandBuffer) {
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(
      ImGui::GetDrawData(),
      static_cast<VulkanCommandBuffer&>(commandBuffer).getHandle()
    );
    ImGui::EndFrame();
    VulkanRenderPassBackend::end(commandBuffer);
}

#define ICON_MIN_FA 0xe005
#define ICON_MAX_FA 0xf8ff

void VulkanImguiRenderPassBackend::loadFonts(const std::string& fontsPath) {
    // TODO: make it configurable
    auto imguiFonts = ImGui::GetIO().Fonts;
    auto handle     = imguiFonts->AddFontFromFileTTF(
      fmt::format("{}/Roboto-Regular.ttf", fontsPath).c_str(), 15
    );
    log::expect(handle, "Could not load font");

    ImFontConfig config;
    config.MergeMode        = true;
    config.GlyphMinAdvanceX = 12.0f;
    config.GlyphMaxAdvanceX = 15.0f;
    config.GlyphOffset.y    = 1.0f;

    static std::array<ImWchar, 3> mergedRanges = { ICON_MIN_FA, ICON_MAX_FA, 0 };

    log::expect(
      imguiFonts->AddFontFromFileTTF(
        fmt::format("{}/fa-solid-900.ttf", fontsPath).c_str(), 13.0f, &config,
        mergedRanges.data()
      ),
      "Could not merge font"
    );
    ImGui_ImplVulkan_CreateFontsTexture();
}

void VulkanImguiRenderPassBackend::setStyles() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors    = style.Colors;

    colors[ImGuiCol_Text]                  = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]              = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg]               = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_PopupBg]               = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border]                = ImVec4(0.40f, 0.40f, 0.40f, 0.50f);
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]               = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_TitleBg]               = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgActive]         = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.10f, 0.10f, 0.10f, 0.75f);
    colors[ImGuiCol_MenuBarBg]             = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark]             = ImVec4(0.25f, 0.90f, 0.65f, 1.00f);
    colors[ImGuiCol_SliderGrab]            = ImVec4(0.28f, 0.56f, 0.84f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.37f, 0.61f, 0.89f, 1.00f);
    colors[ImGuiCol_Button]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_ButtonHovered]         = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonActive]          = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
    colors[ImGuiCol_Header]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
    colors[ImGuiCol_Separator]             = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_SeparatorActive]       = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_ResizeGrip]            = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
    colors[ImGuiCol_Tab]                   = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TabHovered]            = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
    colors[ImGuiCol_TabActive]             = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
    colors[ImGuiCol_TabUnfocused]          = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines]             = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.25f, 0.90f, 0.65f, 0.43f);
    colors[ImGuiCol_DragDropTarget]        = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]          = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.10f, 0.10f, 0.10f, 0.35f);

    style.WindowRounding = 5.0f;
    style.FrameRounding  = 3.0f;
    style.PopupRounding  = 4.0f;
}

}  // namespace sl::vk
