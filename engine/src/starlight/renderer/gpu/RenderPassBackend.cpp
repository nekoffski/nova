#include "RenderPassBackend.hh"

#include "starlight/core/Globals.hh"

#ifdef SL_USE_VK
#include "vulkan/VulkanDevice.hh"
#include "vulkan/VulkanRenderPassBackend.hh"
#endif

namespace sl {

UniquePointer<RenderPassBackend> RenderPassBackend::create(
  const Properties& props, bool hasPreviousPass, bool hasNextPass
) {
#ifdef SL_USE_VK
    auto& device = static_cast<vk::VulkanDevice&>(Device::get().getImpl());

    return props.type == Type::normal
             ? UniquePointer<vk::VulkanRenderPassBackend>::create(
                 device, props, hasPreviousPass, hasNextPass
               )
             : UniquePointer<vk::VulkanImguiRenderPassBackend>::create(
                 device, props, hasPreviousPass, hasNextPass,
                 Globals::get().getConfig().paths.fonts
               );
#else
    log::panic("GPU API vendor not specified");
#endif
}

}  // namespace sl