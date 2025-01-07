#pragma once

#include <optional>
#include <span>

#include "Vulkan.hh"
#include "fwd.hh"

#include "starlight/renderer/gpu/Texture.hh"

#include "VKPhysicalDevice.hh"
#include "VKContext.hh"
#include "VKBuffer.hh"
#include "VKCommandBuffer.hh"

namespace sl::vk {

class VKImage {
public:
    explicit VKImage(
      VKContext& context, VKLogicalDevice&, const Texture::ImageData& imageData
    );

    explicit VKImage(
      VKContext& context, VKLogicalDevice&, const Texture::ImageData& imageData,
      VkImage handle
    );

    ~VKImage();

    void recreate(const Texture::ImageData& imageData, VkImage handle);
    void recreate(const Texture::ImageData& imageData);

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
    Texture::ImageData m_imageData;
    VkImage m_handle;
    VkDeviceMemory m_memory;
    VkImageView m_view;

    // sometimes we want to just wrap an image owned by the swapchain
    bool m_destroyImage;
};

}  // namespace sl::vk
