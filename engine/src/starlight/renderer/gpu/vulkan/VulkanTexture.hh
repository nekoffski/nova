#pragma once

#include <string>
#include <vector>

#include <stb.h>

#include "starlight/core/memory/Memory.hh"
#include "starlight/renderer/gpu/Texture.hh"

#include "Vulkan.hh"
#include "VulkanBuffer.hh"
#include "fwd.hh"

#include "VulkanCommandBuffer.hh"

namespace sl::vk {

class VulkanTextureBase : public Texture {
public:
    explicit VulkanTextureBase(
      VulkanDevice& device, const ImageData& imageData,
      const SamplerProperties& sampler
    );

    VkImageView getView() const;
    VkSampler getSampler() const;

protected:
    void createSampler();
    void createView();

    VulkanDevice& m_device;
    VkImage m_image;
    VkSampler m_sampler;
    VkImageView m_view;
};

class VulkanTexture : public VulkanTextureBase {
public:
    explicit VulkanTexture(
      VulkanDevice& device, const ImageData& imageData,
      const SamplerProperties& sampler
    );

    ~VulkanTexture() override;

    void resize(u32 width, u32 height) override;
    void write(std::span<u8> pixels, CommandBuffer* buffer = nullptr) override;

    VkImageLayout getLayout() const;

private:
    void create();
    void destroy();
    void recreate(const Texture::ImageData& imageData);
    void createImage();
    void allocateAndBindMemory();

    void copyFromBuffer(VulkanBuffer& buffer, VulkanCommandBuffer& commandBuffer);

    void transitionLayout(
      VulkanCommandBuffer& commandBuffer, VkImageLayout oldLayout,
      VkImageLayout newLayout
    );

    VkDeviceMemory m_memory;
    VkImageLayout m_layout;
};

class VulkanSwapchainTexture : public VulkanTextureBase {
public:
    explicit VulkanSwapchainTexture(
      VulkanDevice& device, VkImage handle, const ImageData& imageData,
      const SamplerProperties& sampler
    );

    ~VulkanSwapchainTexture() override;

    void resize(u32 width, u32 height) override;
    void write(std::span<u8> pixels, CommandBuffer* buffer = nullptr) override;
};

}  // namespace sl::vk
