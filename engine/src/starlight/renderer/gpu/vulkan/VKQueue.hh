#pragma once

#include "starlight/renderer/gpu/Queue.hh"

#include "Vulkan.hh"

namespace sl::vk {

class VKQueue : public Queue {
public:
    explicit VKQueue(VkQueue handle);

    bool submit(const SubmitInfo& submitInfo) override;
    bool present(const PresentInfo& presentInfo) override;

private:
    VkQueue m_handle;
};

}  // namespace sl::vk