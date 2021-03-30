#pragma once

#include <memory>

#include "sl/ecs/ComponentView.hpp"
#include "sl/gfx/buffer/VertexArray.h"
#include "sl/gfx/fwd.h"
#include "sl/platform/fwd.h"
#include "sl/scene/components/ParticleEffectComponent.h"
#include "sl/scene/components/TransformComponent.h"

namespace sl::gfx::renderer {

class ParticleEffectRenderer {
public:
    explicit ParticleEffectRenderer(std::shared_ptr<gfx::LowLevelRenderer> renderer);

    void renderParticleEffects(ecs::ComponentView<scene::components::ParticleEffectComponent> pfxs,
        ecs::ComponentView<scene::components::TransformComponent> transforms, std::shared_ptr<gfx::camera::Camera> camera);

private:
    void beginParticleEffect(std::shared_ptr<gfx::camera::Camera>);
    void renderParticle();
    void endParticleEffect();

    std::shared_ptr<gfx::buffer::VertexArray> m_vao;
   
    std::shared_ptr<gfx::Shader> m_shader;
    std::shared_ptr<gfx::LowLevelRenderer> m_renderer;
};
}