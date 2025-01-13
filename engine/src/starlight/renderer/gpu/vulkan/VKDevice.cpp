#include "VKDevice.hh"

#include <optional>

#include <kc/core/Utils.hpp>

#include "starlight/core/Log.hh"
#include "starlight/core/window/glfw/Vulkan.hh"

namespace sl::vk {
VKDevice::VKDevice(Window& window, const Config& config) :
    m_allocator(nullptr), m_instance(config, m_allocator),
#ifdef SL_VK_DEBUG
    m_debugMessenger(m_instance.get(), m_allocator),
#endif
    m_surface(m_instance.get(), window, m_allocator),
    m_physicalDevice(m_instance.get(), m_surface.get()),
    m_logicalDevice(m_physicalDevice.get(), m_allocator) {
}

VKDevice::~VKDevice() {}

VkSurfaceKHR VKDevice::getSurface() { return VkSurfaceKHR(); }

VkInstance VKDevice::getInstance() { return VkInstance(); }

VkAllocationCallbacks* VKDevice::getAllocator() { return m_allocator; }

/*
    VKInstance
*/

static VkApplicationInfo createApplicationInfo(const Config& config) {
    VkApplicationInfo applicationInfo;
    clearMemory(&applicationInfo);
    applicationInfo.sType            = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.apiVersion       = VK_API_VERSION_1_3;
    applicationInfo.pApplicationName = config.window.name.c_str();
    applicationInfo.pEngineName      = "Starlight";
    applicationInfo.engineVersion    = VK_MAKE_VERSION(
      config.version.major, config.version.minor, config.version.build
    );

    return applicationInfo;
}

std::vector<const char*> getExtensions() {
    const auto platformRequiredExtensions = glfw::getRequiredExtensions();

    std::vector<const char*> requiredExtensions = {
#ifdef SL_VK_DEBUG
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
    };

    requiredExtensions.insert(
      requiredExtensions.end(), platformRequiredExtensions.begin(),
      platformRequiredExtensions.end()
    );

    return requiredExtensions;
}

static void assertLayers(std::span<const char*> layers) {
    auto layersAvailable = ::vk::enumerateInstanceLayerProperties();

    std::vector<const char*> layerNames{ layersAvailable.size(), nullptr };
    std::ranges::transform(
      layersAvailable, std::begin(layerNames),
      [](auto& layer) -> const char* { return layer.layerName; }
    );

    LOG_TRACE("Available Vulkan layers:");
    for (const auto& layerName : layerNames) LOG_TRACE("\t{}", layerName);

    for (const auto& requiredLayer : layers) {
        ASSERT(
          kc::core::contains(layerNames, requiredLayer),
          "Required layer {} not found", requiredLayer
        );
        LOG_DEBUG("Layer {} found", requiredLayer);
    }
}

static std::vector<const char*> getLayers() {
    std::vector<const char*> layers;

#ifdef SL_VK_DEBUG
    layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    assertLayers(layers);
    LOG_DEBUG("All required vulkan layers found");
    return layers;
}

static VkInstanceCreateInfo createInstanceCreateInfo(
  const VkApplicationInfo& applicationInfo, std::span<const char*> extensions,
  std::span<const char*> layers
) {
    VkInstanceCreateInfo instanceCreateInfo;
    clearMemory(&instanceCreateInfo);
    instanceCreateInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledExtensionCount   = extensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
    instanceCreateInfo.enabledLayerCount       = layers.size();
    instanceCreateInfo.ppEnabledLayerNames     = layers.data();

    return instanceCreateInfo;
}

VKDevice::VKInstance::VKInstance(const Config& config, Allocator* allocator) :
    m_allocator(allocator), m_handle(VK_NULL_HANDLE) {
    auto applicationInfo = createApplicationInfo(config);
    auto layers          = getLayers();
    auto extensions      = getExtensions();

    for (auto& layer : layers) LOG_DEBUG("Will enable layer: {}", layer);
    for (auto& extension : extensions)
        LOG_DEBUG("Will enable extensions: {}", extension);

    auto instanceCreateInfo =
      createInstanceCreateInfo(applicationInfo, extensions, layers);

    VK_ASSERT(vkCreateInstance(&instanceCreateInfo, m_allocator, &m_handle));
    LOG_TRACE("Vulkan Instance initialized");
}

VKDevice::VKInstance::~VKInstance() {
    if (m_handle) {
        LOG_TRACE("Destroying vulkan instance");
        vkDestroyInstance(m_handle, m_allocator);
    }
}

VkInstance VKDevice::VKInstance::get() { return m_handle; }

/*
    VKDebugMessenger
*/

static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessengerCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
  [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageTypes,
  VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
  [[maybe_unused]] void* pUserData
) {
    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            LOG_WARN("VK_DEBUG_LAYER - {}", pCallbackData->pMessage);

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            LOG_ERROR("VK_DEBUG_LAYER - {}", pCallbackData->pMessage);

        default:
            LOG_INFO("VK_DEBUG_LAYER - {}", pCallbackData->pMessage);
    }

    return false;
}

static VkDebugUtilsMessengerCreateInfoEXT createDebugMessengerCreateInfo() {
    uint32_t logSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    clearMemory(&debugCreateInfo);
    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.messageSeverity = logSeverity;
    debugCreateInfo.messageType =
      VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debugCreateInfo.pfnUserCallback = debugMessengerCallback;

    return debugCreateInfo;
}

VKDevice::VKDebugMessenger::VKDebugMessenger(
  VkInstance instance, Allocator* allocator
) : m_instance(instance), m_allocator(allocator) {
    static const auto debugFactoryFunctionName = "vkCreateDebugUtilsMessengerEXT";

    auto debugCreateInfo      = createDebugMessengerCreateInfo();
    auto createDebugMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(m_instance, debugFactoryFunctionName)
    );
    ASSERT(createDebugMessenger, "Failed to create debug messenger factory");
    VK_ASSERT(
      createDebugMessenger(m_instance, &debugCreateInfo, m_allocator, &m_handle)
    );
    LOG_TRACE("Created Vulkan Debug Messenger");
}

VKDevice::VKDebugMessenger::~VKDebugMessenger() {
    if (m_handle) {
        static const auto debugDestructorFunctionName =
          "vkDestroyDebugUtilsMessengerEXT";

        auto destroyDebugMessenger =
          reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(m_instance, debugDestructorFunctionName)
          );
        destroyDebugMessenger(m_instance, m_handle, m_allocator);
    }
}

/*
    VKSurface
*/

VKDevice::VKSurface::VKSurface(
  VkInstance instance, Window& window, Allocator* allocator
) :
    m_instance(instance), m_allocator(allocator),
    m_handle(glfw::createVulkanSurface(instance, window.getHandle(), allocator)) {
    LOG_TRACE("Vulkan surface created");
}

VKDevice::VKSurface::~VKSurface() {
    if (m_handle) {
        LOG_TRACE("Vulkan surface destroyed");
        vkDestroySurfaceKHR(m_instance, m_handle, m_allocator);
    }
}

VkSurfaceKHR VKDevice::VKSurface::get() { return m_handle; }

/*
    Physical Device
*/

static std::vector<VkPhysicalDevice> getPhysicalDevices(VkInstance instance) {
    u32 deviceCount = 0;
    VK_ASSERT(vkEnumeratePhysicalDevices(instance, &deviceCount, 0));

    ASSERT(deviceCount > 0, "Could not find any physical device");

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    VK_ASSERT(
      vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data())
    );

    return physicalDevices;
}

static bool assignQueues(
  VkPhysicalDevice device, VkSurfaceKHR surface, VKDevice::QueueType requiredQueues,
  VKDevice::Physical::Info& info
) {
    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, 0);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
      device, &queueFamilyCount, queueFamilies.data()
    );

    VKDevice::QueueType foundQueues = VKDevice::QueueType::none;

    const auto markIndex = [&](VKDevice::QueueType type, u32 index) {
        info.queueIndices[type] = index;
        foundQueues |= type;
    };

    for (u32 i = 0; i < queueFamilyCount; ++i) {
        const auto& queueFlags = queueFamilies[i].queueFlags;
        if (queueFlags & VK_QUEUE_GRAPHICS_BIT)
            markIndex(VKDevice::QueueType::graphics, i);

        if (queueFlags & VK_QUEUE_COMPUTE_BIT)
            markIndex(VKDevice::QueueType::compute, i);

        if (queueFlags & VK_QUEUE_TRANSFER_BIT)
            markIndex(VKDevice::QueueType::transfer, i);

        VkBool32 supportsPresent = false;
        VK_ASSERT(
          vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supportsPresent)
        );

        if (supportsPresent) markIndex(VKDevice::QueueType::present, i);
    }
    return (foundQueues & requiredQueues) == requiredQueues;
}

static bool queryDeviceSwapchainSupport(
  VkPhysicalDevice device, VkSurfaceKHR surface, VKDevice::Physical::Info& info
) {
    VK_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      device, surface, &info.surfaceCapabilities
    ));

    u32 count = 0;
    VK_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, 0));

    if (count == 0) {
        LOG_INFO("No surface formats supported, skipping");
        return false;
    }

    info.surfaceFormats.resize(count);
    VK_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(
      device, surface, &count, info.surfaceFormats.data()
    ));

    count = 0;
    VK_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, 0));

    if (count == 0) {
        LOG_INFO("No surface present modes supported, skipping");
        return false;
    }

    info.presentModes.resize(count);
    VK_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(
      device, surface, &count, info.presentModes.data()
    ));

    return true;
}

static bool validateExtensions(
  VkPhysicalDevice device, std::span<const char* const> extensions
) {
    if (extensions.empty()) return true;

    u32 availableExtensionCount = 0;
    std::vector<VkExtensionProperties> availableExtenions;

    VK_ASSERT(
      vkEnumerateDeviceExtensionProperties(device, 0, &availableExtensionCount, 0)
    );
    availableExtenions.resize(availableExtensionCount);

    VK_ASSERT(vkEnumerateDeviceExtensionProperties(
      device, 0, &availableExtensionCount, availableExtenions.data()
    ));

    std::vector<const char*> availableExtensionsNames;
    availableExtenions.reserve(availableExtensionCount);

    std::ranges::transform(
      availableExtenions, std::back_inserter(availableExtensionsNames),
      [](const auto& extension) -> const char* { return extension.extensionName; }
    );

    for (const auto& requiredExtension : extensions) {
        if (not kc::core::contains(availableExtensionsNames, requiredExtension)) {
            LOG_INFO("Extension {} not available", requiredExtension);
            return false;
        }
    }

    return true;
}

static std::optional<VKDevice::Physical::Info> getPhysicalDeviceInfo(
  VkPhysicalDevice device, VkSurfaceKHR surface,
  const VKDevice::Physical::Requirements& requirements
) {
    VKDevice::Physical::Info info;

    vkGetPhysicalDeviceProperties(device, &info.coreProperties);
    vkGetPhysicalDeviceMemoryProperties(device, &info.memoryProperties);
    vkGetPhysicalDeviceFeatures(device, &info.features);

    if (requirements.isDiscrete
        && info.coreProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        LOG_INFO("Device is not a discrete GPU, and one is required, skipping");
        return {};
    }

    if (not assignQueues(device, surface, requirements.supportedQueues, info)) {
        LOG_INFO("Could not satisfy queue requirements, skipping");
        return {};
    }

    if (not queryDeviceSwapchainSupport(device, surface, info)) {
        LOG_INFO("Could not satisfy swapchain requirements, skipping");
        return {};
    }

    if (not validateExtensions(device, requirements.extensions)) {
        LOG_WARN("Device doesn't provide required extensions, skipping");
        return {};
    }

    if (requirements.supportsSamplerAnisotropy && !info.features.samplerAnisotropy) {
        LOG_INFO("Device does not support samplerAnisotropy, skipping");
        return {};
    }

    return info;
}

static void showDeviceType(const VkPhysicalDeviceType& type) {
    switch (type) {
        default:
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            LOG_INFO("GPU type is Unknown");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            LOG_INFO("GPU type is Integrated");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            LOG_INFO("GPU type is Discrete");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            LOG_INFO("GPU type is Virtual");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            LOG_INFO("GPU type is CPU");
            break;
    }
}

static void showDeviceInfo(const VKDevice::Physical::Info& info) {
    LOG_INFO("Selected device: '{}'.", info.coreProperties.deviceName);
    showDeviceType(info.coreProperties.deviceType);

    LOG_INFO(
      "GPU Driver version: {}.{}.{}",
      VK_VERSION_MAJOR(info.coreProperties.driverVersion),
      VK_VERSION_MINOR(info.coreProperties.driverVersion),
      VK_VERSION_PATCH(info.coreProperties.driverVersion)
    );

    LOG_INFO(
      "Vulkan API version: {}.{}.{}",
      VK_VERSION_MAJOR(info.coreProperties.apiVersion),
      VK_VERSION_MINOR(info.coreProperties.apiVersion),
      VK_VERSION_PATCH(info.coreProperties.apiVersion)
    );

    for (uint32_t j = 0; j < info.memoryProperties.memoryHeapCount; ++j) {
        const float memorySize =
          (static_cast<float>(info.memoryProperties.memoryHeaps[j].size) / 1024.0f
           / 1024.0f / 1024.0f);
        if (info.memoryProperties.memoryHeaps[j].flags
            & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
            LOG_INFO("Local GPU memory: {} GiB", memorySize);
        } else {
            LOG_INFO("Shared System memory: {} GiB", memorySize);
        }
    }
}

VKDevice::Physical::Physical(VkInstance instance, VkSurfaceKHR surface) :
    m_handle(VK_NULL_HANDLE) {
    Requirements requirements{
        .supportedQueues =
          QueueType::graphics | QueueType::present | QueueType::transfer,
        .isDiscrete                = true,
        .supportsSamplerAnisotropy = true,
        .extensions                = { VK_KHR_SWAPCHAIN_EXTENSION_NAME }
    };

    for (const auto device : getPhysicalDevices(instance)) {
        if (auto info = getPhysicalDeviceInfo(device, surface, requirements); info) {
            showDeviceInfo(*info);
            m_info   = *info;
            m_handle = device;
            break;
        }
    }

    ASSERT(m_handle != VK_NULL_HANDLE, "Could not select suitable physical device");
    LOG_TRACE("Physical device found and initialized");
}

VkPhysicalDevice VKDevice::Physical::get() { return m_handle; }

/*
    Logical Device
*/

VKDevice::Logical::Logical(VkPhysicalDevice device, Allocator* allocator) :
    m_handle(VK_NULL_HANDLE), m_allocator(allocator) {}

VKDevice::Logical::~Logical() {
    LOG_TRACE("Destroying vulkan logical device");
    if (m_graphicsCommandPool)
        vkDestroyCommandPool(m_handle, m_graphicsCommandPool, m_allocator);
    if (m_handle) vkDestroyDevice(m_handle, m_allocator);
}

VkDevice VKDevice::Logical::get() { return m_handle; }

}  // namespace sl::vk
