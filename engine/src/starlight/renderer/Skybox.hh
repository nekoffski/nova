#pragma once

#include "starlight/core/Resource.hh"
#include "starlight/core/memory/Memory.hh"

#include "gpu/Texture.hh"
#include "Mesh.hh"
#include "starlight/renderer/gpu/Shader.hh"

namespace sl {

class Skybox : public NonMovable, public Identificable<Skybox> {
public:
    explicit Skybox(SharedPointer<Texture> cubeMap);
    ~Skybox();

    Texture* getCubeMap();

private:
    SharedPointer<Texture> m_cubeMap;
};

}  // namespace sl
