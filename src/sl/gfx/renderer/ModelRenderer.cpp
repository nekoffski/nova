#include "ModelRenderer.h"

#include "sl/core/Profiler.h"
#include "sl/gfx/LowLevelRenderer.h"
#include "sl/scene/components/MaterialComponent.h"
#include "sl/scene/components/ModelComponent.h"
#include "sl/scene/components/RendererComponent.h"
#include "sl/scene/components/TransformComponent.h"

namespace sl::gfx::renderer {

const scene::components::MaterialComponent DEFAULT_MATERIAL = scene::components::MaterialComponent();

ModelRenderer::ModelRenderer(std::shared_ptr<gfx::LowLevelRenderer> renderer)
    : m_renderer(renderer) {
}

void ModelRenderer::render(scene::components::RendererComponent& component, ecs::ComponentView<scene::components::MaterialComponent> materials,
    ecs::ComponentView<scene::components::ModelComponent> models, ecs::ComponentView<scene::components::TransformComponent> transforms,
    std::shared_ptr<gfx::camera::Camera> camera, std::shared_ptr<gfx::Shader> shader) {

    shader->setUniform("view", camera->getViewMatrix());
    shader->setUniform("viewPos", camera->getPosition());

    auto& entityId = component.ownerEntityId;

    // TODO: POSSIBLE BRANCHING
    auto& material = materials.doesEntityOwnComponent(entityId) ? materials.getByEntityId(entityId) : DEFAULT_MATERIAL;

    setMaterial(material, shader);

    auto& model = models.getByEntityId(entityId);
    auto transformComponent = transforms.getByEntityId(entityId);

    renderModelComposite(shader, model.modelData, transformComponent.transformation, camera);
}

void ModelRenderer::render(scene::components::RendererComponent& component, ecs::ComponentView<scene::components::MaterialComponent> materials,
    ecs::ComponentView<scene::components::ModelComponent> models, ecs::ComponentView<scene::components::TransformComponent> transforms,
    std::shared_ptr<gfx::camera::Camera> camera) {

    render(component, materials, models, transforms, camera, component.shader);
}

void ModelRenderer::setMaterial(const scene::components::MaterialComponent& material, std::shared_ptr<gfx::Shader> shader) {
    shader->setUniform("material.ambientColor", material.ambientColor);
    shader->setUniform("material.diffuseColor", material.diffuseColor);
    shader->setUniform("material.specularColor", material.specularColor);
    shader->setUniform("material.shininess", material.shininess);
}

void ModelRenderer::renderModelComposite(std::shared_ptr<gfx::Shader> shader, const gfx::data::ModelData& modelData,
    const math::Mat4& transform, std::shared_ptr<gfx::camera::Camera> camera) {

    float t = core::Clock::now()->value();
    shader->setUniform("t", t);
    shader->setUniform("projection", camera->getProjectionMatrix());

    for (const auto& position : modelData.positions) {
        shader->setUniform("model", transform * math::translate(position));
        renderModel(modelData.model);
    }
}

void ModelRenderer::renderModel(std::shared_ptr<geom::Model> model) {
    for (const auto& mesh : model->meshes)
        renderMesh(mesh);
}

void ModelRenderer::renderMesh(std::shared_ptr<geom::Mesh> mesh) {
    int i = 0;
    for (const auto& texture : mesh->textures)
        texture->bind(i++);

    auto& vao = mesh->vertexArray;

    vao->bind();
    m_renderer->renderVertexArray(vao);
    vao->unbind();

    for (const auto& texture : mesh->textures)
        texture->unbind();
}
}