#include "Swapchain.hh"

#ifdef SL_USE_VK
#include "vulkan/VulkanDevice.hh"
#include "vulkan/VulkanSwapchain.hh"
#endif

namespace sl {

UniquePointer<Swapchain> Swapchain::create(const Vec2<u32>& size) {
#ifdef SL_USE_VK
    return UniquePointer<vk::VulkanSwapchain>::create(
      static_cast<vk::VulkanDevice&>(Device::get().getImpl()), size
    );
#else
    log::panic("GPU API vendor not specified");
#endif
}

}  // namespace sl
