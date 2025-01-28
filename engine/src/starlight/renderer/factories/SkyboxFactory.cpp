#include "SkyboxFactory.hh"

#include "starlight/renderer/gpu/Texture.hh"
#include "starlight/renderer/factories/ShaderFactory.hh"
#include "starlight/renderer/factories/TextureFactory.hh"

namespace sl {

SkyboxFactory::SkyboxFactory() :
    ResourceFactory("Skybox"),
    m_defaultSkyboxShader(ShaderFactory::get().load("Builtin.Shader.Skybox")) {}

ResourceRef<Skybox> SkyboxFactory::load(const std::string& name) {
    if (auto resource = find(name); resource) [[unlikely]]
        return resource;

    auto cubemap = TextureFactory::get().load(name, Texture::Type::cubemap);
    return store(name, UniquePointer<Skybox>::create(cubemap));
}

ResourceRef<Shader> SkyboxFactory::getDefaultShader() {
    return m_defaultSkyboxShader;
}

}  // namespace sl
