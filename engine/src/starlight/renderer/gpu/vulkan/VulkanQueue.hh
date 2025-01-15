#pragma once

#include "starlight/renderer/gpu/Queue.hh"

#include "Vulkan.hh"

namespace sl::vk {

class VulkanQueue : public Queue {
public:
    explicit VulkanQueue(VkQueue handle);

    void wait() override;
    bool submit(const SubmitInfo& submitInfo) override;
    bool present(const PresentInfo& presentInfo) override;

private:
    VkQueue m_handle;
};

}  // namespace sl::vk