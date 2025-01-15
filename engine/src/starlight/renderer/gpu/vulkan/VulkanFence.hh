#pragma once

#include "Vulkan.hh"
#include "fwd.hh"

#include "starlight/renderer/gpu/Sync.hh"

namespace sl::vk {

class VulkanFence : public Fence {
public:
    explicit VulkanFence(VulkanDevice& device, State state);
    ~VulkanFence();

    bool wait(Nanoseconds timeout) override;
    void reset() override;

    VkFence getHandle();

private:
    VkFence m_handle;
    VulkanDevice& m_device;
    State m_state;
    Allocator* m_allocator;
};

VulkanFence& toVk(Fence&);

}  // namespace sl::vk
