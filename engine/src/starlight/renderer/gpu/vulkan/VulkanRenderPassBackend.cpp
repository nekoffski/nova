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

    VK_ASSERT(vkCreateFramebuffer(
      m_device.logical.handle, &createInfo, m_device.allocator, &handle
    ));
}

VulkanRenderPassBackend::Framebuffer::~Framebuffer() {
    if (handle)
        vkDestroyFramebuffer(m_device.logical.handle, handle, m_device.allocator);
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
    LOG_TRACE("Creating VulkanRenderPassBackend instance");

    for (auto& target : properties.renderTargets) {
        if (target.colorAttachment != nullptr) m_hasColorAttachment = true;
        if (target.depthAttachment != nullptr) m_hasDepthAttachment = true;
    }

    VulkanRenderPassBackendCreateInfo createInfo(
      m_device.physical.info.depthFormat, m_device.physical.info.surfaceFormat,
      properties, hasPreviousPass, hasNextPass, m_hasColorAttachment,
      m_hasDepthAttachment
    );

    VK_ASSERT(vkCreateRenderPass(
      m_device.logical.handle, &createInfo.handle, m_device.allocator, &m_handle
    ));
    LOG_TRACE("Vulkan render pass handle created");

    if (properties.renderTargets.size() == 0)
        LOG_WARN("Render pass with no render targets created");

    generateRenderTargets();
    LOG_TRACE("VulkanRenderPassBackend instance created");
}

VulkanRenderPassBackend::~VulkanRenderPassBackend() {
    if (m_handle) {
        m_device.waitIdle();
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
        m_framebuffers.emplace_back(
          m_device, m_handle, m_props.rect.size, attachmentViews
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
    VkDescriptorPoolSize poolSizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER,                1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1000 }
    };
    VkDescriptorPoolCreateInfo poolInfo;
    poolInfo.pNext         = nullptr;
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets       = 1000;
    poolInfo.poolSizeCount = std::size(poolSizes);
    poolInfo.pPoolSizes    = poolSizes;

    VK_ASSERT(vkCreateDescriptorPool(
      m_device.logical.handle, &poolInfo, m_device.allocator, &m_uiPool
    ));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(
      static_cast<GLFWwindow*>(m_device.window.getHandle()), true
    );

    ImGui_ImplVulkan_InitInfo initInfo;
    clearMemory(&initInfo);
    initInfo.Instance       = m_device.instance.handle;
    initInfo.PhysicalDevice = m_device.physical.handle;
    initInfo.Device         = m_device.logical.handle;
    initInfo.Queue          = m_device.getQueue(Queue::Type::graphics).getHandle();
    initInfo.DescriptorPool = m_uiPool;
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

    if (m_uiPool) {
        vkDestroyDescriptorPool(
          m_device.logical.handle, m_uiPool, m_device.allocator
        );
    }
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
    ASSERT(handle, "Could not load font");

    ImFontConfig config;
    config.MergeMode        = true;
    config.GlyphMinAdvanceX = 12.0f;
    config.GlyphMaxAdvanceX = 15.0f;
    config.GlyphOffset.y    = 1.0f;

    static std::array<ImWchar, 3> mergedRanges = { ICON_MIN_FA, ICON_MAX_FA, 0 };

    ASSERT(
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

    // light style from Pacôme Danhiez (user itamago)
    // https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
    style.Alpha                    = 0.99f;
    style.WindowRounding           = 0.0f;
    style.FrameRounding            = 0.0f;
    style.WindowMenuButtonPosition = ImGuiDir_None;

    auto& colors              = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

    // Headers
    colors[ImGuiCol_Header]        = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
    colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
    colors[ImGuiCol_HeaderActive]  = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

    // Buttons
    colors[ImGuiCol_Button]        = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
    colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
    colors[ImGuiCol_ButtonActive]  = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

    // Frame BG
    colors[ImGuiCol_FrameBg]        = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
    colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
    colors[ImGuiCol_FrameBgActive]  = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

    // Tabs
    colors[ImGuiCol_Tab]                = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    colors[ImGuiCol_TabHovered]         = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
    colors[ImGuiCol_TabActive]          = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
    colors[ImGuiCol_TabUnfocused]       = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

    // Title
    colors[ImGuiCol_TitleBg]          = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    colors[ImGuiCol_TitleBgActive]    = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
}

}  // namespace sl::vk
