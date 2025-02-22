#pragma once

#include "starlight/core/Factory.hh"
#include "starlight/renderer/Skybox.hh"

namespace sl {

class SkyboxFactory : public Factory<SkyboxFactory, Skybox> {
public:
    explicit SkyboxFactory();

    SharedPointer<Skybox> load(const std::string& name);
    SharedPointer<Shader> getDefaultShader();

private:
    SharedPointer<Shader> m_defaultSkyboxShader;
};

}  // namespace sl
