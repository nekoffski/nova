#pragma once

#include "starlight/core/utils/Resource.hh"
#include "starlight/core/memory/Memory.hh"

#include "gpu/Texture.hh"
#include "Mesh.hh"
#include "starlight/renderer/gpu/Shader.hh"

namespace sl {

class Skybox : public NonMovable, public Identificable<Skybox> {
public:
    explicit Skybox(ResourceRef<Texture> cubeMap);
    ~Skybox();

    Texture* getCubeMap();

    void applyUniforms(
      ResourceRef<Shader> shader, CommandBuffer& commandBuffer, u32 imageIndex
    );

private:
    ResourceRef<Texture> m_cubeMap;
    LocalPtr<Shader::Instance> m_instance;
};

}  // namespace sl
