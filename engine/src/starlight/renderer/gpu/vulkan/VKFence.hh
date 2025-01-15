#pragma once

#include "Vulkan.hh"
#include "fwd.hh"

#include "starlight/renderer/gpu/Sync.hh"

namespace sl::vk {

class VKFence : public Fence {
public:
    explicit VKFence(VkDevice device, Allocator* allocator, State state);
    ~VKFence();

    bool wait(Nanoseconds timeout) override;
    void reset() override;

    VkFence getHandle();

private:
    VkFence m_handle;
    VkDevice m_device;
    Allocator* m_allocator;
};

VKFence& toVk(Fence&);

}  // namespace sl::vk
