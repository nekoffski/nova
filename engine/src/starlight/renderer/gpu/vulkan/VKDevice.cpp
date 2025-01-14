#include "VKDevice.hh"

#include <optional>

#include <kc/core/Utils.hpp>

#include "VKQueue.hh"

#include "starlight/core/Log.hh"
#include "starlight/core/window/glfw/Vulkan.hh"

#include "VKSemaphore.hh"

namespace sl::vk {
VKDevice::VKDevice(Window& window, const Config& config) :
    m_allocator(nullptr), m_instance(config, m_allocator),
#ifdef SL_VK_DEBUG
    m_debugMessenger(m_instance.handle, m_allocator),
#endif
    m_surface(m_instance.handle, window, m_allocator),
    m_physicalDevice(m_instance.handle, m_surface.handle),
    m_logicalDevice(
      m_physicalDevice.handle, m_allocator, m_physicalDevice.info.queueIndices
    ) {
}

VKDevice::~VKDevice() {}

VkSurfaceKHR VKDevice::getSurface() { return VkSurfaceKHR(); }

OwningPtr<Fence> VKDevice::createFence(Fence::State) { return OwningPtr<Fence>(); }

OwningPtr<Swapchain> VKDevice::createSwapchain(const Vec2<u32>& size) {
    return OwningPtr<Swapchain>();
}

OwningPtr<Semaphore> VKDevice::createSemaphore() {
    return createOwningPtr<v2::VKSemaphore>(m_logicalDevice.handle, m_allocator);
}

VkInstance VKDevice::getInstance() { return VkInstance(); }

VkAllocationCallbacks* VKDevice::getAllocator() { return m_allocator; }

void VKDevice::waitIdle() { vkDeviceWaitIdle(m_logicalDevice.handle); }

Queue& VKDevice::getQueue(Queue::Type type) {
    return m_logicalDevice.queues.at(type);
}

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
    handle(VK_NULL_HANDLE), m_allocator(allocator) {
    auto applicationInfo = createApplicationInfo(config);
    auto layers          = getLayers();
    auto extensions      = getExtensions();

    for (auto& layer : layers) LOG_DEBUG("Will enable layer: {}", layer);
    for (auto& extension : extensions)
        LOG_DEBUG("Will enable extensions: {}", extension);

    auto instanceCreateInfo =
      createInstanceCreateInfo(applicationInfo, extensions, layers);

    VK_ASSERT(vkCreateInstance(&instanceCreateInfo, m_allocator, &handle));
    LOG_TRACE("Vulkan Instance initialized");
}

VKDevice::VKInstance::~VKInstance() {
    if (handle) {
        LOG_TRACE("Destroying vulkan instance");
        vkDestroyInstance(handle, m_allocator);
    }
}

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
    handle(glfw::createVulkanSurface(instance, window.getHandle(), allocator)),
    m_instance(instance), m_allocator(allocator) {
    LOG_TRACE("Vulkan surface created");
}

VKDevice::VKSurface::~VKSurface() {
    if (handle) {
        LOG_TRACE("Vulkan surface destroyed");
        vkDestroySurfaceKHR(m_instance, handle, m_allocator);
    }
}

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
  VkPhysicalDevice device, VkSurfaceKHR surface, Queue::Type requiredQueues,
  VKDevice::Physical::Info& info
) {
    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, 0);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
      device, &queueFamilyCount, queueFamilies.data()
    );

    Queue::Type foundQueues = Queue::Type::none;

    const auto markIndex = [&](Queue::Type type, u32 index) {
        info.queueIndices[type] = index;
        foundQueues |= type;
    };

    for (u32 i = 0; i < queueFamilyCount; ++i) {
        const auto& queueFlags = queueFamilies[i].queueFlags;
        if (queueFlags & VK_QUEUE_GRAPHICS_BIT) markIndex(Queue::Type::graphics, i);
        if (queueFlags & VK_QUEUE_COMPUTE_BIT) markIndex(Queue::Type::compute, i);
        if (queueFlags & VK_QUEUE_TRANSFER_BIT) markIndex(Queue::Type::transfer, i);

        VkBool32 supportsPresent = false;
        VK_ASSERT(
          vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supportsPresent)
        );

        if (supportsPresent) markIndex(Queue::Type::present, i);
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

static bool detectDepthFormat(
  VkPhysicalDevice device, VKDevice::Physical::Info& info
) {
    static std::vector<std::pair<VkFormat, u8>> candidates = {
        { VK_FORMAT_D32_SFLOAT,         4 },
        { VK_FORMAT_D32_SFLOAT_S8_UINT, 4 },
        { VK_FORMAT_D24_UNORM_S8_UINT,  3 },
    };

    uint32_t flags   = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    info.depthFormat = VK_FORMAT_UNDEFINED;

    for (const auto& [format, channelCount] : candidates) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(device, format, &properties);

        if (((properties.linearTilingFeatures & flags) == flags)
            || ((properties.optimalTilingFeatures & flags) == flags)) {
            info.depthChannelCount = format;
            info.depthChannelCount = channelCount;
            return true;
        }
    }

    return false;
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
        LOG_INFO("Device doesn't provide required extensions, skipping");
        return {};
    }

    if (requirements.supportsSamplerAnisotropy && !info.features.samplerAnisotropy) {
        LOG_INFO("Device does not support samplerAnisotropy, skipping");
        return {};
    }

    if (not detectDepthFormat(device, info)) {
        LOG_INFO("Could not detect depth format, skipping");
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
    handle(VK_NULL_HANDLE) {
    Requirements requirements{
        .supportedQueues =
          Queue::Type::graphics | Queue::Type::present | Queue::Type::transfer,
        .isDiscrete                = true,
        .supportsSamplerAnisotropy = true,
        .extensions                = { VK_KHR_SWAPCHAIN_EXTENSION_NAME }
    };

    for (const auto device : getPhysicalDevices(instance)) {
        if (auto info = getPhysicalDeviceInfo(device, surface, requirements); info) {
            showDeviceInfo(*info);
            this->info = *info;
            handle     = device;
            break;
        }
    }

    ASSERT(handle != VK_NULL_HANDLE, "Could not select suitable physical device");
    LOG_TRACE("Physical device found and initialized");
}

/*
    Logical Device
*/

VKDevice::Logical::Logical(
  VkPhysicalDevice device, Allocator* allocator,
  const Physical::QueueIndices& queueIndices
) :
    handle(VK_NULL_HANDLE), m_physicalDevice(device), m_allocator(allocator),
    m_graphicsCommandPool(VK_NULL_HANDLE) {
    createDevice(queueIndices);
    assignQueues(queueIndices);
    createCommandPool(queueIndices);

    LOG_TRACE("Vulkan logical device created");
}

VKDevice::Logical::~Logical() {
    LOG_TRACE("Destroying vulkan logical device");
    if (m_graphicsCommandPool)
        vkDestroyCommandPool(handle, m_graphicsCommandPool, m_allocator);
    if (handle) vkDestroyDevice(handle, m_allocator);
}

void VKDevice::Logical::createDevice(const Physical::QueueIndices& queueIndices) {
    static constexpr u64 maximumExpectedQueuesCount = 3;

    std::vector<u32> indices;
    indices.reserve(maximumExpectedQueuesCount);
    indices.push_back(queueIndices.at(Queue::Type::graphics));

    if (queueIndices.at(Queue::Type::graphics)
        != queueIndices.at(Queue::Type::present)) {
        indices.push_back(queueIndices.at(Queue::Type::present));
    }
    if (queueIndices.at(Queue::Type::graphics)
        != queueIndices.at(Queue::Type::transfer)) {
        indices.push_back(queueIndices.at(Queue::Type::transfer));
    }

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::vector<f32> queueProrities;

    auto queueCount = indices.size();
    queueProrities.reserve(queueCount);
    queueCreateInfos.reserve(queueCount);

    for (const auto index : indices) {
        LOG_TRACE("Adding queue family index: {}", index);

        queueProrities.push_back(1.0f);
        VkDeviceQueueCreateInfo info;
        clearMemory(&info);
        info.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueFamilyIndex = index;
        info.queueCount       = 1;
        info.flags            = 0;
        info.pQueuePriorities = &queueProrities.back();
        queueCreateInfos.push_back(info);
    }

    VkPhysicalDeviceFeatures deviceFeatures;
    clearMemory(&deviceFeatures);
    deviceFeatures.samplerAnisotropy        = VK_TRUE;
    std::vector<const char*> extensionNames = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo deviceCreateInfo;
    clearMemory(&deviceCreateInfo);
    deviceCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount    = queueCreateInfos.size();
    deviceCreateInfo.pQueueCreateInfos       = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures        = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount   = extensionNames.size();
    deviceCreateInfo.ppEnabledExtensionNames = extensionNames.data();

    VK_ASSERT(
      vkCreateDevice(m_physicalDevice, &deviceCreateInfo, m_allocator, &handle)
    );
}

void VKDevice::Logical::createCommandPool(const Physical::QueueIndices& queueIndices
) {
    VkCommandPoolCreateInfo poolCreateInfo;
    clearMemory(&poolCreateInfo);
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    poolCreateInfo.queueFamilyIndex = queueIndices.at(Queue::Type::graphics);
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VK_ASSERT(vkCreateCommandPool(
      handle, &poolCreateInfo, m_allocator, &m_graphicsCommandPool
    ));
}

void VKDevice::Logical::assignQueues(const Physical::QueueIndices& queueIndices) {
    const auto types = {
        Queue::Type::graphics, Queue::Type::present, Queue::Type::transfer
    };
    for (auto type : types) {
        VkQueue queue;
        vkGetDeviceQueue(handle, queueIndices.at(type), 0, &queue);
        queues.emplace(type, queue);
    }
}

}  // namespace sl::vk
