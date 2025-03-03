#pragma once

#include "starlight/core/Factory.hh"
#include "starlight/core/FileSystem.hh"
#include "starlight/renderer/Material.hh"

namespace sl {

class MaterialFactory : public Factory<MaterialFactory, Material> {
public:
    inline static auto defaultDiffuseColor = Vec4<f32>{ 1.0f };
    inline static auto defaultShininess    = 32.0f;

    explicit MaterialFactory();

    SharedPtr<Material> load(
      const std::string& name, const FileSystem& fs = FileSystem::getDefault()
    );

    SharedPtr<Material> create(
      const std::string& name, const Material::Properties& properties
    );

    SharedPtr<Material> getDefault();

private:
    void createDefault();

    SharedPtr<Material> m_defaultMaterial;
};

}  // namespace sl
