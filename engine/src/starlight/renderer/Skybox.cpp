#include "Skybox.hh"

#include "starlight/renderer/gpu/Texture.hh"
#include "starlight/renderer/factories/ShaderFactory.hh"
#include "starlight/renderer/factories/TextureFactory.hh"

namespace sl {

Skybox::Skybox(ResourceRef<Texture> cubeMap) : m_cubeMap(cubeMap) {}

Skybox::~Skybox() { log::trace("Destroying Skybox: {}", getId()); }

Texture* Skybox::getCubeMap() { return m_cubeMap.get(); }

}  // namespace sl
