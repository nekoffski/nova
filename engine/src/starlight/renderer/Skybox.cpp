#include "Skybox.hh"

#include "starlight/renderer/gpu/Texture.hh"

namespace sl {

Skybox::Skybox(SharedPtr<Texture> cubeMap, OptStr name) :
    NamedResource(name), m_cubeMap(cubeMap) {}

Texture* Skybox::getCubeMap() { return m_cubeMap.get(); }

}  // namespace sl
