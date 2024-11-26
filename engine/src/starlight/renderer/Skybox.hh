#pragma once

#include "starlight/core/utils/Resource.hh"

#include "gpu/Texture.hh"
#include "gpu/Mesh.hh"
#include "gpu/Shader.hh"

namespace sl {

class Skybox : public NonMovable, public Identificable<Skybox> {
public:
    explicit Skybox(ResourceRef<Texture> cubeMap, ResourceRef<Shader> shader);
    ~Skybox();

    Texture* getCubeMap();
    Shader* getShader();
    u32 getInstanceId() const;

    static ResourceRef<Skybox> load(const std::string& name);

private:
    ResourceRef<Texture> m_cubeMap;
    ResourceRef<Shader> m_shader;

    u32 m_instanceId;
};

class SkyboxManager
    : public ResourceManager<Skybox>,
      public kc::core::Singleton<SkyboxManager> {
public:
    explicit SkyboxManager();

    ResourceRef<Skybox> load(const std::string& name);

private:
    ResourceRef<Shader> m_defaultSkyboxShader;
};

}  // namespace sl
