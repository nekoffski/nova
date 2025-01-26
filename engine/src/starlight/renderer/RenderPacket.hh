#pragma once

#include "starlight/core/math/Core.hh"

#include "gpu/Texture.hh"
#include "Mesh.hh"
#include "light/PointLight.hh"
#include "light/DirectionalLight.hh"
#include "camera/Camera.hh"
#include "Material.hh"
#include "Skybox.hh"

namespace sl {

struct RenderEntity {
    Mat4<f32> worldTransform;
    Mesh* mesh;
    Material* material;
};

struct RenderPacket {
    Skybox* skybox;
    Camera* camera;
    std::vector<PointLight> pointLights;
    std::vector<DirectionalLight> directionalLights;
    std::vector<RenderEntity> entities;
    std::vector<Texture*> shadowMaps;  // TODO: static array instead
    u64 frameNumber;
};

}  // namespace sl
