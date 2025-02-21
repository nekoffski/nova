#include "Buffer.hh"

#include "Device.hh"

#ifdef SL_USE_VK
#include "vulkan/VulkanDevice.hh"
#include "vulkan/VulkanBuffer.hh"
#endif

namespace sl {

UniquePointer<Buffer> Buffer::create(const Properties& props) {
#ifdef SL_USE_VK
    return UniquePointer<vk::VulkanBuffer>::create(
      static_cast<vk::VulkanDevice&>(Device::get().getImpl()), props
    );
#else
    log::panic("GPU API vendor not specified");
#endif
}

}  // namespace sl