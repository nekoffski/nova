#include "VKUIRenderer.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>
#include <ImGuizmo.h>

#include "starlight/core/utils/Log.h"

#include "Vulkan.h"
#include "VKDevice.h"
#include "VKContext.h"
#include "VKRenderPass.h"

namespace sl::vk {

VKUIRenderer::VKUIRenderer(
  VKBackendAccessor& backendAccessor, RendererBackendProxy& backendProxy,
  Window& window, RenderPass* renderPass
) :
    m_context(*backendAccessor.getContext()),
    m_device(*backendAccessor.getLogicalDevice()), m_backendProxy(backendProxy) {
    ASSERT(not s_hasInstance, "Only single instance of UI renderer is allowed");
    s_hasInstance = true;

    VkDescriptorPoolSize poolSizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER,                 1000},
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000},
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000},
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1000},
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1000},
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000},
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000},
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1000}
    };
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets       = 1000;
    poolInfo.poolSizeCount = std::size(poolSizes);
    poolInfo.pPoolSizes    = poolSizes;

    VK_ASSERT(vkCreateDescriptorPool(
      m_device.getHandle(), &poolInfo, m_context.getAllocator(), &m_uiPool
    ));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow*>(window.getHandle()), true);

    auto graphicsQueue    = m_device.getQueues().graphics;
    auto renderPassHandle = static_cast<VKRenderPass*>(renderPass)->getHandle();

    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance                  = m_context.getInstance();
    initInfo.PhysicalDevice = backendAccessor.getPhysicalDevice()->getHandle();
    initInfo.Device         = m_device.getHandle();
    initInfo.Queue          = graphicsQueue;
    initInfo.DescriptorPool = m_uiPool;
    initInfo.MinImageCount  = 3;
    initInfo.ImageCount     = 3;
    initInfo.MSAASamples    = VK_SAMPLE_COUNT_1_BIT;
    initInfo.RenderPass     = renderPassHandle;

    ImGui_ImplVulkan_Init(&initInfo);

    VKUIRenderer::reloadFontTextures();

    LOG_TRACE("UI backend initialized successfully");
}

void VKUIRenderer::reloadFontTextures() { ImGui_ImplVulkan_CreateFontsTexture(); }

VKUIRenderer::~VKUIRenderer() {
    m_device.waitIdle();

    if (m_uiPool) {
        vkDestroyDescriptorPool(
          m_device.getHandle(), m_uiPool, m_context.getAllocator()
        );
    }

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void VKUIRenderer::begin([[maybe_unused]] CommandBuffer& commandBuffer) {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
}

void VKUIRenderer::end(CommandBuffer& commandBuffer) {
    ImGui::Render();
    auto bufferHandle =
      static_cast<vk::VKCommandBuffer*>(&commandBuffer)->getHandle();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), bufferHandle);
    ImGui::EndFrame();
}

}  // namespace sl::vk
