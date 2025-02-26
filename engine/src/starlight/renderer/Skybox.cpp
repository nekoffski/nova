#include "Skybox.hh"

#include "starlight/renderer/gpu/Texture.hh"

namespace sl {

Skybox::Skybox(SharedPtr<Texture> cubeMap) : m_cubeMap(cubeMap) {}

Skybox::~Skybox() { log::trace("Destroying Skybox: {}", id); }

Texture* Skybox::getCubeMap() { return m_cubeMap.get(); }

}  // namespace sl
