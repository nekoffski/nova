#include "Skybox.hh"

#include "starlight/renderer/gpu/Texture.hh"
#include "starlight/renderer/factories/ShaderFactory.hh"
#include "starlight/renderer/factories/TextureFactory.hh"

namespace sl {

Skybox::Skybox(ResourceRef<Texture> cubeMap) : m_cubeMap(cubeMap) {}

Skybox::~Skybox() { log::trace("Destroying Skybox: {}", getId()); }

void Skybox::applyUniforms(
  Shader& shader, CommandBuffer& commandBuffer, u32 imageIndex
) {
    // shader.setLocalUniforms(
    //   commandBuffer, m_shaderUniformOffset, imageIndex,
    //   [&](Shader::UniformProxy& proxy) { proxy.set("cubeTexture",
    //   m_cubeMap.get()); }
    // );
}

Texture* Skybox::getCubeMap() { return m_cubeMap.get(); }

}  // namespace sl
