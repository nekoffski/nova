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
#include "VKContext.hh"
#include "VKPhysicalDevice.hh"
#include "VKContext.hh"

#include "VKCommandBuffer.hh"

namespace sl::vk {

class VKTexture : public Texture {
public:
    explicit VKTexture(
      VKContext& context, VKLogicalDevice& device, const ImageData& imageData,
      const SamplerProperties& sampler
    );

    // TODO: consider implementing separated VKSwapchainTexture
    explicit VKTexture(
      VKContext& context, VKLogicalDevice& device, VkImage handle,
      const ImageData& imageData, const SamplerProperties& sampler
    );

    ~VKTexture() override;

    const VKImage* getImage() const;
    VkSampler getSampler() const;

    void resize(u32 width, u32 height, VkImage handle);
    void resize(u32 width, u32 height) override;

    void write(u32 offset, std::span<u8> pixels) override;

private:
    void createSampler(const SamplerProperties& props);

    VKContext& m_context;
    VKLogicalDevice& m_device;

    VKImage m_image;
    VkSampler m_sampler;

    u32 m_generation;
};

}  // namespace sl::vk
