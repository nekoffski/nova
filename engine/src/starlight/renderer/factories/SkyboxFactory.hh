#pragma once

#include "starlight/core/Resource.hh"
#include "starlight/core/memory/Memory.hh"

#include "starlight/renderer/Skybox.hh"

namespace sl {

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
