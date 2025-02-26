#include "SkyboxFactory.hh"

#include "ShaderFactory.hh"
#include "TextureFactory.hh"

namespace sl {

SkyboxFactory::SkyboxFactory() :
    m_defaultSkyboxShader(ShaderFactory::get().load("Builtin.Shader.Skybox")) {}

SharedPtr<Skybox> SkyboxFactory::load(const std::string& name) {
    if (auto resource = find(name); resource) [[unlikely]]
        return resource;

    auto cubemap = TextureFactory::get().load(name, Texture::Type::cubemap);
    return save(name, SharedPtr<Skybox>::create(cubemap));
}

SharedPtr<Shader> SkyboxFactory::getDefaultShader() { return m_defaultSkyboxShader; }

}  // namespace sl