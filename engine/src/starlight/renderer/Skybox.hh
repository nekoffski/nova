#pragma once

#include "starlight/core/utils/Resource.hh"

#include "gpu/Texture.hh"
#include "gpu/Mesh.hh"
#include "gpu/Shader.hh"

namespace sl {

class Skybox : public NonMovable, public Identificable<Skybox> {
public:
    explicit Skybox(ResourceRef<Texture> cubeMap);
    ~Skybox();

    Texture* getCubeMap();

    void applyUniforms(Shader& shader, CommandBuffer& commandBuffer, u8 imageIndex);

    static ResourceRef<Skybox> load(const std::string& name);
    static Shader* getDefaultShader();

private:
    ResourceRef<Texture> m_cubeMap;
    ShaderInstanceMap m_shaderInstanceMap;
};

class SkyboxManager
    : public ResourceManager<Skybox>,
      public kc::core::Singleton<SkyboxManager> {
public:
    explicit SkyboxManager();

    ResourceRef<Skybox> load(const std::string& name);
    Shader* getDefaultShader();

private:
    ResourceRef<Shader> m_defaultSkyboxShader;
};

}  // namespace sl
