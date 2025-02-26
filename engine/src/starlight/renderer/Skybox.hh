#pragma once

#include "starlight/core/memory/Memory.hh"

#include "gpu/Texture.hh"
#include "Mesh.hh"
#include "starlight/renderer/gpu/Shader.hh"

namespace sl {

class Skybox : public NonMovable, public NamedResource<Skybox, "Skybox"> {
public:
    explicit Skybox(SharedPtr<Texture> cubeMap);
    ~Skybox();

    Texture* getCubeMap();

private:
    SharedPtr<Texture> m_cubeMap;
};

}  // namespace sl
