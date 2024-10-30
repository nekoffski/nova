#pragma once

#include <optional>
#include <span>

#include "Vulkan.hh"
#include "fwd.hh"

#include "VKPhysicalDevice.hh"
#include "VKContext.hh"
#include "VKBuffer.hh"
#include "VKCommandBuffer.hh"

namespace sl::vk {

class VKImage {
public:
    enum class Type : u8 { flat, cubemap };

    struct Properties {
        Type type;
        u32 width;
        u32 height;
        VkFormat format;
        VkImageTiling tiling;
        VkImageUsageFlags usage;
        VkMemoryPropertyFlags memoryFlags;
        bool createView;
        VkImageAspectFlags viewAspectFlags;
        u8 channels;
    };

    explicit VKImage(
      VKContext& context, VKLogicalDevice&, const Properties& properties
    );
    explicit VKImage(
      VKContext& context, VKLogicalDevice&, const Properties& properties,
      std::span<const u8> pixels
    );
    explicit VKImage(
      VKContext& context, VKLogicalDevice&, const Properties& properties,
      VkImage handle
    );

    ~VKImage();

    void recreate(const Properties& properties, VkImage handle);
    void recreate(const Properties& properties);

    const Properties& getProperties() const;

    VKImage(const VKImage& oth)            = delete;
    VKImage& operator=(const VKImage& oth) = delete;
    VKImage& operator=(VKImage&& oth)      = delete;
    VKImage(VKImage&&)                     = delete;

    void write(u32 offset, std::span<const u8> pixels);

    VkImageView getView() const;

private:
    void copyFromBuffer(VKBuffer& buffer, VKCommandBuffer& commandBuffer);

    void transitionLayout(
      VKCommandBuffer& commandBuffer, VkFormat format, VkImageLayout oldLayout,
      VkImageLayout newLayout
    );

    void create();
    void destroy();

    void createImage();
    void allocateAndBindMemory();
    void createView();

    VKContext& m_context;
    VKLogicalDevice& m_device;

    Properties m_props;

    VkImage m_handle;
    VkDeviceMemory m_memory;
    VkImageView m_view;

    // sometimes we want to just wrap an image owned by the swapchain
    bool m_destroyImage;
};

}  // namespace sl::vk
