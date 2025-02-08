#include "VulkanDevice.hh"

#include <optional>

#include "VulkanQueue.hh"

#include "starlight/core/Log.hh"
#include "starlight/core/window/glfw/Vulkan.hh"
#include "starlight/core/event/WindowResized.hh"

#include "VulkanFence.hh"
#include "VulkanSemaphore.hh"
#include "VulkanSwapchain.hh"
#include "VulkanCommandBuffer.hh"
#include "VulkanTexture.hh"
#include "VulkanShader.hh"
#include "VulkanRenderPassBackend.hh"
#include "VulkanPipeline.hh"
#include "VulkanBuffer.hh"
#include "VulkanShaderDataBinder.hh"

namespace sl::vk {

VulkanDevice::VulkanDevice(Context& context) :
    window(context.getWindow()), config(context.getConfig()),
    m_eventSentinel(context.getEventProxy()), allocator(nullptr),
    instance(config, allocator),
#ifdef SL_VK_DEBUG
    m_debugMessenger(instance.handle, allocator),
#endif
    surface(instance.handle, window, allocator),
    physical(instance.handle, surface.handle),
    logical(physical.handle, allocator, physical.info.queueIndices) {

    createUiResources();

    m_eventSentinel.add<WindowResized>([&]([[maybe_unused]] auto&) {
        onWindowResize();
    });
}

VulkanDevice::~VulkanDevice() {
    if (uiDescriptorPool) {
        vkDestroyDescriptorPool(logical.handle, uiDescriptorPool, allocator);
    }
}

UniquePointer<Buffer> VulkanDevice::createBuffer(const Buffer::Properties& props) {
    return UniquePointer<VulkanBuffer>::create(*this, props);
}

UniquePointer<Texture> VulkanDevice::createTexture(
  const Texture::ImageData& image, const Texture::SamplerProperties& sampler
) {
    return UniquePointer<VulkanTexture>::create(*this, image, sampler);
}

UniquePointer<Fence> VulkanDevice::createFence(Fence::State state) {
    return UniquePointer<VulkanFence>::create(*this, state);
}

UniquePointer<Swapchain> VulkanDevice::createSwapchain(const Vec2<u32>& size) {
    return UniquePointer<VulkanSwapchain>::create(*this, size);
}

UniquePointer<RenderPassBackend> VulkanDevice::createRenderPassBackend(
  const RenderPassBackend::Properties& props, bool hasPreviousPass, bool hasNextPass
) {
    return props.type == RenderPassBackend::Type::normal
             ? UniquePointer<VulkanRenderPassBackend>::create(
                 *this, props, hasPreviousPass, hasNextPass
               )
             : UniquePointer<VulkanImguiRenderPassBackend>::create(
                 *this, props, hasPreviousPass, hasNextPass, config.paths.fonts
               );
}

UniquePointer<Semaphore> VulkanDevice::createSemaphore() {
    return UniquePointer<VulkanSemaphore>::create(*this);
}

UniquePointer<CommandBuffer> VulkanDevice::createCommandBuffer(
  CommandBuffer::Severity severity
) {
    return UniquePointer<VulkanCommandBuffer>::create(*this, severity);
}

UniquePointer<Shader> VulkanDevice::createShader(const Shader::Properties& props) {
    return UniquePointer<VulkanShader>::create(*this, props);
}

UniquePointer<Pipeline> VulkanDevice::createPipeline(
  Shader& shader, RenderPassBackend& renderPass, const Pipeline::Properties& props
) {
    return UniquePointer<VulkanPipeline>::create(
      *this, static_cast<VulkanShader&>(shader),
      static_cast<VulkanRenderPassBackend&>(renderPass), props
    );
}

UniquePointer<ShaderDataBinder> VulkanDevice::createShaderDataBinder(Shader& shader
) {
    return UniquePointer<VulkanShaderDataBinder>::create(
      *this, static_cast<VulkanShader&>(shader)
    );
}

void VulkanDevice::waitIdle() { vkDeviceWaitIdle(logical.handle); }

VulkanQueue& VulkanDevice::getQueue(Queue::Type type) {
    return logical.queues.at(type);
}

std::optional<i32> VulkanDevice::findMemoryIndex(u32 typeFilter, u32 propertyFlags)
  const {
    const auto& props = physical.info.memoryProperties;
    for (u32 i = 0; i < props.memoryTypeCount; ++i) {
        bool isSuitable =
          (typeFilter & (1 << i))
          && (props.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags;
        if (isSuitable) return i;
    }
    log::warn(
      "Unable to find suitable memory type: {}/{}", typeFilter, propertyFlags
    );
    return {};
}

/*
    Instance
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

    log::trace("Available Vulkan layers:");
    for (const auto& layerName : layerNames) log::trace("\t{}", layerName);

    for (const auto& requiredLayer : layers) {
        log::expect(
          contains(layerNames, requiredLayer), "Required layer {} not found",
          requiredLayer
        );
        log::debug("Layer {} found", requiredLayer);
    }
}

static std::vector<const char*> getLayers() {
    std::vector<const char*> layers;

#ifdef SL_VK_DEBUG
    layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    assertLayers(layers);
    log::debug("All required vulkan layers found");
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

VulkanDevice::Instance::Instance(const Config& config, Allocator* allocator) :
    handle(VK_NULL_HANDLE), m_allocator(allocator) {
    auto applicationInfo = createApplicationInfo(config);
    auto layers          = getLayers();
    auto extensions      = getExtensions();

    for (auto& layer : layers) log::debug("Will enable layer: {}", layer);
    for (auto& extension : extensions)
        log::debug("Will enable extensions: {}", extension);

    auto instanceCreateInfo =
      createInstanceCreateInfo(applicationInfo, extensions, layers);

    log::expect(vkCreateInstance(&instanceCreateInfo, m_allocator, &handle));
    log::trace("Vulkan Instance initialized");
}

VulkanDevice::Instance::~Instance() {
    if (handle) {
        log::trace("vkDestroyInstance: {}", static_cast<void*>(handle));
        vkDestroyInstance(handle, m_allocator);
    }
}

/*
    DebugMessenger
*/

static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessengerCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
  [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageTypes,
  VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
  [[maybe_unused]] void* pUserData
) {
    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            log::warn("VK_DEBUG_LAYER - {}", pCallbackData->pMessage);
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            log::error("VK_DEBUG_LAYER - {}", pCallbackData->pMessage);
            break;

        default:
            log::info("VK_DEBUG_LAYER - {}", pCallbackData->pMessage);
            break;
    }

    return false;
}

static VkDebugUtilsMessengerCreateInfoEXT createDebugMessengerCreateInfo() {
    u32 logSeverity =
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

VulkanDevice::DebugMessenger::DebugMessenger(
  VkInstance instance, Allocator* allocator
) : m_instance(instance), m_allocator(allocator) {
    static const auto debugFactoryFunctionName = "vkCreateDebugUtilsMessengerEXT";

    auto debugCreateInfo      = createDebugMessengerCreateInfo();
    auto createDebugMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(m_instance, debugFactoryFunctionName)
    );
    log::expect(createDebugMessenger, "Failed to create debug messenger factory");
    log::expect(
      createDebugMessenger(m_instance, &debugCreateInfo, m_allocator, &m_handle)
    );
    log::trace("Created Vulkan Debug Messenger");
}

VulkanDevice::DebugMessenger::~DebugMessenger() {
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
    Surface
*/

VulkanDevice::Surface::Surface(
  VkInstance instance, Window& window, Allocator* allocator
) :
    handle(glfw::createVulkanSurface(instance, window.getHandle(), allocator)),
    m_instance(instance), m_allocator(allocator) {
    log::trace("Vulkan surface created");
}

VulkanDevice::Surface::~Surface() {
    if (handle) {
        log::trace("vkDestroySurfaceKHR: {}", static_cast<void*>(handle));
        vkDestroySurfaceKHR(m_instance, handle, m_allocator);
    }
}

/*
    Physical Device
*/

static std::vector<VkPhysicalDevice> getPhysicalDevices(VkInstance instance) {
    u32 deviceCount = 0;
    log::expect(vkEnumeratePhysicalDevices(instance, &deviceCount, 0));
    log::expect(deviceCount > 0, "Could not find any physical device");

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    log::expect(
      vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data())
    );

    return physicalDevices;
}

static bool assignQueues(
  VkPhysicalDevice device, VkSurfaceKHR surface, Queue::Type requiredQueues,
  VulkanDevice::Physical::Info& info
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
        log::expect(
          vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supportsPresent)
        );

        if (supportsPresent) markIndex(Queue::Type::present, i);
    }
    return (foundQueues & requiredQueues) == requiredQueues;
}

static bool queryDeviceSwapchainSupport(
  VkPhysicalDevice device, VkSurfaceKHR surface, VulkanDevice::Physical::Info& info
) {
    log::expect(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      device, surface, &info.surfaceCapabilities
    ));

    u32 count = 0;
    log::expect(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, 0));

    if (count == 0) {
        log::info("No surface formats supported, skipping");
        return false;
    }

    info.surfaceFormats.resize(count);
    log::expect(vkGetPhysicalDeviceSurfaceFormatsKHR(
      device, surface, &count, info.surfaceFormats.data()
    ));

    count = 0;
    log::expect(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, 0)
    );

    if (count == 0) {
        log::info("No surface present modes supported, skipping");
        return false;
    }

    info.presentModes.resize(count);
    log::expect(vkGetPhysicalDeviceSurfacePresentModesKHR(
      device, surface, &count, info.presentModes.data()
    ));

    return true;
}

void VulkanDevice::onWindowResize() {
    queryDeviceSwapchainSupport(physical.handle, surface.handle, physical.info);
}

void VulkanDevice::createUiResources() {
    VkDescriptorPoolSize poolSizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER,                1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1000 }
    };
    VkDescriptorPoolCreateInfo poolInfo;
    poolInfo.pNext         = nullptr;
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets       = 1000;
    poolInfo.poolSizeCount = std::size(poolSizes);
    poolInfo.pPoolSizes    = poolSizes;

    log::expect(
      vkCreateDescriptorPool(logical.handle, &poolInfo, allocator, &uiDescriptorPool)
    );
}

static bool validateExtensions(
  VkPhysicalDevice device, std::span<const char* const> extensions
) {
    if (extensions.empty()) return true;

    u32 availableExtensionCount = 0;
    std::vector<VkExtensionProperties> availableExtenions;

    log::expect(
      vkEnumerateDeviceExtensionProperties(device, 0, &availableExtensionCount, 0)
    );
    availableExtenions.resize(availableExtensionCount);

    log::expect(vkEnumerateDeviceExtensionProperties(
      device, 0, &availableExtensionCount, availableExtenions.data()
    ));

    std::vector<const char*> availableExtensionsNames;
    availableExtenions.reserve(availableExtensionCount);

    std::ranges::transform(
      availableExtenions, std::back_inserter(availableExtensionsNames),
      [](const auto& extension) -> const char* { return extension.extensionName; }
    );

    for (const auto& requiredExtension : extensions) {
        if (not contains(availableExtensionsNames, requiredExtension)) {
            log::info("Extension {} not available", requiredExtension);
            return false;
        }
    }

    return true;
}

static void pickSurfaceFormat(VulkanDevice::Physical::Info& deviceInfo) {
    static const auto demandedFormat     = VK_FORMAT_R8G8B8A8_UNORM;
    static const auto demandedColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    deviceInfo.surfaceFormat             = deviceInfo.surfaceFormats[0];

    for (const auto& format : deviceInfo.surfaceFormats) {
        if (format.format == demandedFormat
            && format.colorSpace == demandedColorSpace) {
            deviceInfo.surfaceFormat = format;
            break;
        }
    }
}

static void pickPresentMode(VulkanDevice::Physical::Info& deviceInfo) {
    static const VkPresentModeKHR defaultPresentMode  = VK_PRESENT_MODE_FIFO_KHR;
    static const VkPresentModeKHR demandedPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;

    deviceInfo.presentMode =
      contains(deviceInfo.presentModes, demandedPresentMode)
        ? demandedPresentMode
        : defaultPresentMode;
}

static bool detectDepthFormat(
  VkPhysicalDevice device, VulkanDevice::Physical::Info& info
) {
    static std::vector<std::pair<VkFormat, u8>> candidates = {
        { VK_FORMAT_D32_SFLOAT,         4 },
        { VK_FORMAT_D32_SFLOAT_S8_UINT, 4 },
        { VK_FORMAT_D24_UNORM_S8_UINT,  3 },
    };

    u32 flags        = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    info.depthFormat = VK_FORMAT_UNDEFINED;

    for (const auto& [format, channelCount] : candidates) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(device, format, &properties);

        if (((properties.linearTilingFeatures & flags) == flags)
            || ((properties.optimalTilingFeatures & flags) == flags)) {
            info.depthFormat       = format;
            info.depthChannelCount = channelCount;
            return true;
        }
    }

    return false;
}

static std::optional<VulkanDevice::Physical::Info> getPhysicalDeviceInfo(
  VkPhysicalDevice device, VkSurfaceKHR surface,
  const VulkanDevice::Physical::Requirements& requirements
) {
    VulkanDevice::Physical::Info info;

    vkGetPhysicalDeviceProperties(device, &info.coreProperties);
    vkGetPhysicalDeviceMemoryProperties(device, &info.memoryProperties);
    vkGetPhysicalDeviceFeatures(device, &info.features);

    if (requirements.isDiscrete
        && info.coreProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        log::info("Device is not a discrete GPU, and one is required, skipping");
        return {};
    }

    if (not assignQueues(device, surface, requirements.supportedQueues, info)) {
        log::info("Could not satisfy queue requirements, skipping");
        return {};
    }

    if (not queryDeviceSwapchainSupport(device, surface, info)) {
        log::info("Could not satisfy swapchain requirements, skipping");
        return {};
    }

    if (not validateExtensions(device, requirements.extensions)) {
        log::info("Device doesn't provide required extensions, skipping");
        return {};
    }

    if (requirements.supportsSamplerAnisotropy && !info.features.samplerAnisotropy) {
        log::info("Device does not support samplerAnisotropy, skipping");
        return {};
    }

    if (not detectDepthFormat(device, info)) {
        log::info("Could not detect depth format, skipping");
        return {};
    }

    for (u32 i = 0; i < info.memoryProperties.memoryTypeCount; ++i) {
        if ((info.memoryProperties.memoryTypes[i].propertyFlags
             & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            && (info.memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
            info.supportsDeviceLocalHostVisibleMemory = true;
            break;
        }
    }

    pickSurfaceFormat(info);
    pickPresentMode(info);

    return info;
}

static void showDeviceType(const VkPhysicalDeviceType& type) {
    switch (type) {
        default:
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            log::info("GPU type is Unknown");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            log::info("GPU type is Integrated");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            log::info("GPU type is Discrete");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            log::info("GPU type is Virtual");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            log::info("GPU type is CPU");
            break;
    }
}

static void showDeviceInfo(const VulkanDevice::Physical::Info& info) {
    log::info("Selected device: '{}'.", info.coreProperties.deviceName);
    showDeviceType(info.coreProperties.deviceType);

    log::info(
      "GPU Driver version: {}.{}.{}",
      VK_VERSION_MAJOR(info.coreProperties.driverVersion),
      VK_VERSION_MINOR(info.coreProperties.driverVersion),
      VK_VERSION_PATCH(info.coreProperties.driverVersion)
    );

    log::info(
      "Vulkan API version: {}.{}.{}",
      VK_VERSION_MAJOR(info.coreProperties.apiVersion),
      VK_VERSION_MINOR(info.coreProperties.apiVersion),
      VK_VERSION_PATCH(info.coreProperties.apiVersion)
    );

    for (u32 j = 0; j < info.memoryProperties.memoryHeapCount; ++j) {
        const float memorySize =
          (static_cast<float>(info.memoryProperties.memoryHeaps[j].size) / 1024.0f
           / 1024.0f / 1024.0f);
        if (info.memoryProperties.memoryHeaps[j].flags
            & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
            log::info("Local GPU memory: {} GiB", memorySize);
        } else {
            log::info("Shared System memory: {} GiB", memorySize);
        }
    }
}

VulkanDevice::Physical::Physical(VkInstance instance, VkSurfaceKHR surface) :
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

    log::expect(
      handle != VK_NULL_HANDLE, "Could not select suitable physical device"
    );
    log::trace("Physical device found and initialized");
}

/*
    Logical Device
*/

VulkanDevice::Logical::Logical(
  VkPhysicalDevice device, Allocator* allocator,
  const Physical::QueueIndices& queueIndices
) :
    handle(VK_NULL_HANDLE), graphicsCommandPool(VK_NULL_HANDLE),
    m_physicalDevice(device), m_allocator(allocator) {
    createDevice(queueIndices);
    assignQueues(queueIndices);
    createCommandPool(queueIndices);

    log::trace("Vulkan logical device created");
}

VulkanDevice::Logical::~Logical() {
    vkDeviceWaitIdle(handle);

    if (graphicsCommandPool) {
        log::trace(
          "vkDestroyCommandPool: {}", static_cast<void*>(graphicsCommandPool)
        );
        vkDestroyCommandPool(handle, graphicsCommandPool, m_allocator);
    }
    if (handle) {
        log::trace("vkDestroyDevice: {}", static_cast<void*>(handle));
        vkDestroyDevice(handle, m_allocator);
    }
}

void VulkanDevice::Logical::createDevice(const Physical::QueueIndices& queueIndices
) {
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
        log::trace("Adding queue family index: {}", index);

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

    log::expect(
      vkCreateDevice(m_physicalDevice, &deviceCreateInfo, m_allocator, &handle)
    );
    log::trace("vkCreateDevice: {}", static_cast<void*>(handle));
}

void VulkanDevice::Logical::createCommandPool(
  const Physical::QueueIndices& queueIndices
) {
    VkCommandPoolCreateInfo poolCreateInfo;
    clearMemory(&poolCreateInfo);
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    poolCreateInfo.queueFamilyIndex = queueIndices.at(Queue::Type::graphics);
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    log::expect(
      vkCreateCommandPool(handle, &poolCreateInfo, m_allocator, &graphicsCommandPool)
    );
    log::trace("vkCreateCommandPool: {}", static_cast<void*>(graphicsCommandPool));
}

void VulkanDevice::Logical::assignQueues(const Physical::QueueIndices& queueIndices
) {
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
