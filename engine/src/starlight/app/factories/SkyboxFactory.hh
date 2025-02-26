#pragma once

#include "starlight/core/Factory.hh"
#include "starlight/renderer/Skybox.hh"

namespace sl {

class SkyboxFactory : public Factory<SkyboxFactory, Skybox> {
public:
    explicit SkyboxFactory();

    SharedPtr<Skybox> load(const std::string& name);
    SharedPtr<Shader> getDefaultShader();

private:
    SharedPtr<Shader> m_defaultSkyboxShader;
};

}  // namespace sl
