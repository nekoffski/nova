#include "Sync.hh"

#ifdef SL_USE_VK
#include "vulkan/VulkanDevice.hh"
#include "vulkan/VulkanSemaphore.hh"
#include "vulkan/VulkanFence.hh"
#endif

namespace sl {

UniquePtr<Fence> Fence::create(State state) {
#ifdef SL_USE_VK
    return UniquePtr<vk::VulkanFence>::create(
      static_cast<vk::VulkanDevice&>(Device::get().getImpl()), state
    );
#else
    log::panic("GPU API vendor not specified");
#endif
}

UniquePtr<Semaphore> Semaphore::create() {
#ifdef SL_USE_VK
    return UniquePtr<vk::VulkanSemaphore>::create(
      static_cast<vk::VulkanDevice&>(Device::get().getImpl())
    );
#else
    log::panic("GPU API vendor not specified");
#endif
}

}  // namespace sl
