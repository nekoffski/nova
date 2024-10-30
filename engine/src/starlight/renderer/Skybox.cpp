#include "Skybox.hh"

#include "starlight/renderer/gpu/Texture.hh"

namespace sl {

Skybox::Skybox(
  ResourceRef<Texture> cubeMap, ResourceRef<Mesh> mesh, ResourceRef<Shader> shader
) :
    m_cubeMap(cubeMap), m_mesh(mesh), m_shader(shader),
    m_instanceId(shader->acquireInstanceResources({ cubeMap.get() })) {}

ResourceRef<Skybox> Skybox::load(const std::string& name) {
    return SkyboxManager::get().load(name);
}

Skybox::~Skybox() { m_shader->releaseInstanceResources(m_instanceId); }

Mesh* Skybox::getMesh() { return m_mesh.get(); }

u32 Skybox::getInstanceId() const { return m_instanceId; }

Texture* Skybox::getCubeMap() { return m_cubeMap.get(); }

Shader* Skybox::getShader() { return m_shader.get(); }

SkyboxManager::SkyboxManager() :
    m_cubeMesh(Mesh::getCube()),
    m_defaultSkyboxShader(Shader::load("Builtin.Shader.Skybox")) {}

ResourceRef<Skybox> SkyboxManager::load(const std::string& name) {
    auto texture = Texture::load(name, Texture::Type::cubemap);

    return store(
      name, createOwningPtr<Skybox>(texture, m_cubeMesh, m_defaultSkyboxShader)
    );
}

}  // namespace sl
