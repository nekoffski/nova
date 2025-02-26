#include "Pipeline.hh"

#ifdef SL_USE_VK
#include "vulkan/VulkanDevice.hh"
#include "vulkan/VulkanPipeline.hh"
#include "vulkan/VulkanShader.hh"
#include "vulkan/VulkanRenderPassBackend.hh"
#endif

#include "starlight/window/Window.hh"

namespace sl {

Pipeline::Properties Pipeline::Properties::createDefault() {
    const auto size = Window::get().getSize();

    static Vec2<u32> origin{ 0u, 0u };

    return Properties{
        .viewport         = { origin, size },
        .scissor          = { origin, size },
        .polygonMode      = PolygonMode::fill,
        .cullMode         = CullMode::back,
        .depthTestEnabled = true,
    };
}

UniquePtr<Pipeline> Pipeline::create(
  Shader& shader, RenderPassBackend& renderPass, const Properties& props
) {
#ifdef SL_USE_VK
    return UniquePtr<vk::VulkanPipeline>::create(
      static_cast<vk::VulkanDevice&>(Device::get().getImpl()),
      static_cast<vk::VulkanShader&>(shader),
      static_cast<vk::VulkanRenderPassBackend&>(renderPass), props
    );
#else
    log::panic("GPU API vendor not specified");
#endif
}

}  // namespace sl
