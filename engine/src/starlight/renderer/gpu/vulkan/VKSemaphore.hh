#pragma once

#include "Vulkan.hh"

#include "starlight/renderer/gpu/Sync.hh"

#include "VKLogicalDevice.hh"
#include "VKContext.hh"

#include "fwd.hh"

namespace sl::vk {

namespace v2 {

class VKSemaphore : public Semaphore {
public:
    explicit VKSemaphore(VkDevice device, Allocator* allocator);
    ~VKSemaphore();

    VkSemaphore getHandle();
    VkSemaphore* getHandlePointer();

private:
    VkSemaphore m_handle;
    VkDevice m_device;
    Allocator* m_allocator;
};

}  // namespace v2

class VKSemaphore : public NonCopyable, public NonMovable {
public:
    explicit VKSemaphore(VKContext& context, VKLogicalDevice& device);
    ~VKSemaphore();

    VkSemaphore getHandle() const;
    VkSemaphore* getHandlePointer();

private:
    VKContext& m_context;
    VKLogicalDevice& m_device;

    VkSemaphore m_handle;
};

}  // namespace sl::vk
