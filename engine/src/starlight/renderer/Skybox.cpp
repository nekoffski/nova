#include "Skybox.hh"

#include "starlight/renderer/gpu/Texture.hh"

namespace sl {

Skybox::Skybox(ResourceRef<Texture> cubeMap) : m_cubeMap(cubeMap) {
    m_instance.emplace(std::vector<Texture*>{ m_cubeMap.get() });
}

Skybox::~Skybox() { LOG_TRACE("Destroying Skybox"); }

void Skybox::applyUniforms(
  ResourceRef<Shader> shader, CommandBuffer& commandBuffer, u8 imageIndex
) {
    shader->setInstanceUniforms(
      commandBuffer, m_instance->getId(shader), imageIndex,
      [&](Shader::UniformProxy& proxy) { proxy.set("cubeTexture", m_cubeMap.get()); }
    );
}

Texture* Skybox::getCubeMap() { return m_cubeMap.get(); }

SkyboxManager::SkyboxManager() :
    ResourceManager("Skybox"),
    m_defaultSkyboxShader(ShaderManager::get().load("Builtin.Shader.Skybox")) {}

ResourceRef<Skybox> SkyboxManager::load(const std::string& name) {
    if (auto resource = find(name); resource) [[unlikely]]
        return resource;

    auto cubemap = TextureManager::get().load(name, Texture::Type::cubemap);
    return store(name, createOwningPtr<Skybox>(cubemap));
}

ResourceRef<Shader> SkyboxManager::getDefaultShader() {
    return m_defaultSkyboxShader;
}

}  // namespace sl
