#pragma once

#include "starlight/core/utils/Resource.hh"
#include "starlight/core/memory/Memory.hh"

#include "gpu/Texture.hh"
#include "gpu/Mesh.hh"
#include "gpu/Shader.hh"

namespace sl {

class Skybox : public NonMovable, public Identificable<Skybox> {
public:
    explicit Skybox(ResourceRef<Texture> cubeMap);
    ~Skybox();

    Texture* getCubeMap();

    void applyUniforms(
      ResourceRef<Shader> shader, CommandBuffer& commandBuffer, u32 imageIndex
    );

private:
    ResourceRef<Texture> m_cubeMap;
    LocalPtr<Shader::Instance> m_instance;
};

class SkyboxFactory
    : public ResourceFactory<Skybox>,
      public kc::core::Singleton<SkyboxFactory> {
public:
    explicit SkyboxFactory();

    ResourceRef<Skybox> load(const std::string& name);
    ResourceRef<Shader> getDefaultShader();

private:
    ResourceRef<Shader> m_defaultSkyboxShader;
};

}  // namespace sl
