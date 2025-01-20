#include "Image.hh"

#include "backends/imgui_impl_vulkan.h"
#include "starlight/renderer/gpu/vulkan/VulkanTexture.hh"

namespace sl::ui {

class VulkanImageHandle : public ImageHandle {
public:
    explicit VulkanImageHandle(Texture* texture);

    void show(const Vec2<f32>& size, const Vec2<f32>& minUV, const Vec2<f32>& maxUV)
      override;

private:
    VkDescriptorSet createDescriptorSet(Texture* texture);
    VkDescriptorSet m_descriptorSet;
};

std::unique_ptr<ImageHandle> ImageHandle::createHandle(Texture* texture) {
    return std::make_unique<VulkanImageHandle>(texture);
}

VulkanImageHandle::VulkanImageHandle(Texture* texture
) : m_descriptorSet(createDescriptorSet(texture)) {}

void VulkanImageHandle::show(
  const Vec2<f32>& size, const Vec2<f32>& minUV, const Vec2<f32>& maxUV
) {
    ImGui::Image(
      reinterpret_cast<ImTextureID>(m_descriptorSet), { size.x, size.y },
      { minUV.x, minUV.y }, { maxUV.x, maxUV.y }
    );
}

VkDescriptorSet VulkanImageHandle::createDescriptorSet(Texture* texture) {
    auto VulkanTexture = static_cast<vk::VulkanTexture*>(texture);
    auto sampler       = VulkanTexture->getSampler();
    auto view          = VulkanTexture->getView();

    return ImGui_ImplVulkan_AddTexture(
      sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );
}

}  // namespace sl::ui
