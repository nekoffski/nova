#include "Device.hh"

#ifdef SL_USE_VK
#include "starlight/renderer/gpu/vulkan/VulkanDevice.hh"
#endif

namespace sl {

UniquePointer<Device> Device::create(Context& context) {
#ifdef SL_USE_VK
    return UniquePointer<vk::VulkanDevice>::create(context);
#endif
    return nullptr;
}

Queue& Device::getPresentQueue() { return getQueue(Queue::Type::present); }

Queue& Device::getGraphicsQueue() { return getQueue(Queue::Type::graphics); }

}  // namespace sl
