#pragma once

#include "Vulkan.hh"

#include "VKLogicalDevice.hh"
#include "VKContext.hh"

#include "fwd.hh"

namespace sl::vk {

namespace v2 {

}

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
