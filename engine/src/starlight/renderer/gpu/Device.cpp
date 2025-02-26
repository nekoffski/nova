#include "Device.hh"

#ifdef SL_USE_VK
#include "starlight/renderer/gpu/vulkan/VulkanDevice.hh"
#endif

namespace sl {

Device::Device() : m_impl(Impl::create()) {}

UniquePtr<Device::Impl> Device::Impl::create() {
#ifdef SL_USE_VK
    return UniquePtr<vk::VulkanDevice>::create();
#endif
    return nullptr;
}

Queue& Device::getPresentQueue() { return getQueue(Queue::Type::present); }

Queue& Device::getGraphicsQueue() { return getQueue(Queue::Type::graphics); }

Queue& Device::getQueue(Queue::Type type) { return m_impl->getQueue(type); }

void Device::waitIdle() { m_impl->waitIdle(); }

Device::Impl& Device::getImpl() { return *m_impl; }

}  // namespace sl
