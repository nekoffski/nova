#pragma once

#include "Vulkan.hh"

#include "starlight/core/fwd.hh"
#include "starlight/core/Core.hh"
#include "starlight/core/Config.hh"
#include "starlight/core/utils/RAIIWrapper.hh"

namespace sl::vk {

class VKContext : public NonCopyable, public NonMovable {
public:
    explicit VKContext(sl::Window& window, const Config& config);
    ~VKContext();

    VkAllocationCallbacks* getAllocator() const;  // TODO: not const!
    VkInstance getInstance() const;
    VkSurfaceKHR getSurface() const;

private:
    VkInstance createInstance();
    VkDebugUtilsMessengerEXT createDebugMessenger();

    Config m_config;

    Allocator* m_allocator;
    RAIIWrapper<VkInstance> m_instance;
    RAIIWrapper<VkDebugUtilsMessengerEXT> m_debugMessenger;
    RAIIWrapper<VkSurfaceKHR> m_surface;
};

}  // namespace sl::vk
