#pragma once

#include <string>
#include <vector>

#include <stb.h>

#include <kc/core/Log.h>

#include "starlight/core/memory/Memory.hh"
#include "starlight/renderer/gpu/Texture.hh"

#include "Vulkan.hh"
#include "VKImage.hh"
#include "VKBuffer.hh"

#include "VKCommandBuffer.hh"

namespace sl::vk {

class VKTextureBase : public Texture {
public:
    VKTextureBase(
      VkDevice device, Allocator* allocator, const ImageData& imageData,
      const SamplerProperties& sampler
    );

    VkImageView getView() const;
    VkSampler getSampler() const;

protected:
    void createSampler();

    VkDevice m_device;
    Allocator* m_allocator;
    VkImage m_image;
    VkSampler m_sampler;
    VkImageView m_view;
};

class VKTexture : public VKTextureBase {
public:
    explicit VKTexture(
      VkDevice device, Allocator* allocator, const ImageData& imageData,
      const SamplerProperties& sampler
    );

    ~VKTexture() override;

    void resize(u32 width, u32 height) override;
    void write(std::span<u8> pixels) override;
};

class VKSwapchainTexture : public VKTextureBase {
public:
    explicit VKSwapchainTexture(
      VkDevice device, Allocator* allocator, VkImage handle,
      const ImageData& imageData, const SamplerProperties& sampler
    );

    ~VKSwapchainTexture() override;

    void resize(u32 width, u32 height) override;
    void write(std::span<u8> pixels) override;
};

}  // namespace sl::vk
