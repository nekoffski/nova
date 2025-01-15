#pragma once

#include <string>
#include <vector>

#include <stb.h>

#include <kc/core/Log.h>

#include "starlight/core/memory/Memory.hh"
#include "starlight/renderer/gpu/Texture.hh"

#include "Vulkan.hh"
#include "VKImage.hh"
#include "VulkanBuffer.hh"
#include "fwd.hh"

#include "VulkanCommandBuffer.hh"

namespace sl::vk {

class VKTextureBase : public Texture {
public:
    VKTextureBase(
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

class VKTexture : public VKTextureBase {
public:
    explicit VKTexture(
      VulkanDevice& device, const ImageData& imageData,
      const SamplerProperties& sampler
    );

    ~VKTexture() override;

    void resize(u32 width, u32 height) override;
    void write(std::span<u8> pixels) override;

private:
    VkDeviceMemory m_memory;

    void create();
    void destroy();
    void recreate(const Texture::ImageData& imageData);
    void createImage();
    void allocateAndBindMemory();
};

class VulkanSwapchainTexture : public VKTextureBase {
public:
    explicit VulkanSwapchainTexture(
      VulkanDevice& device, VkImage handle, const ImageData& imageData,
      const SamplerProperties& sampler
    );

    ~VulkanSwapchainTexture() override;

    void resize(u32 width, u32 height) override;
    void write(std::span<u8> pixels) override;
};

}  // namespace sl::vk
