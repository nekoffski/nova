#pragma once

#include "Vulkan.hh"
#include "fwd.hh"

#include "VKLogicalDevice.hh"
#include "VKContext.hh"

namespace sl::vk {

namespace v2 {

}

class VKFence : public NonCopyable, public NonMovable {
public:
    enum class State : unsigned char { signaled, notSignaled };

    explicit VKFence(VKContext& context, VKLogicalDevice& device, State state);
    ~VKFence();

    bool wait(Nanoseconds timeout);
    void reset();

    VkFence getHandle();

private:
    VkFence m_handle;

    VKContext& m_context;
    VKLogicalDevice& m_device;

    State m_state;
};

}  // namespace sl::vk
