#include "Skybox.hh"

#include "starlight/renderer/gpu/Texture.hh"

namespace sl {

Skybox::Skybox(ResourceRef<Texture> cubeMap) : m_cubeMap(cubeMap) {
    m_instance.emplace(std::vector<Texture*>{ m_cubeMap.get() });
}

ResourceRef<Skybox> Skybox::load(const std::string& name) {
    return SkyboxManager::get().load(name);
}

Skybox::~Skybox() { LOG_TRACE("Destroying Skybox"); }

ResourceRef<Shader> Skybox::getDefaultShader() {
    return SkyboxManager::get().getDefaultShader();
}

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
    m_defaultSkyboxShader(Shader::load("Builtin.Shader.Skybox")) {}

ResourceRef<Skybox> SkyboxManager::load(const std::string& name) {
    auto cubemap = Texture::load(name, Texture::Type::cubemap);
    return store(name, createOwningPtr<Skybox>(cubemap));
}

ResourceRef<Shader> SkyboxManager::getDefaultShader() {
    return m_defaultSkyboxShader;
}

}  // namespace sl
