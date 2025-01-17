#pragma once

#include <string>

#include "starlight/core/utils/Resource.hh"
#include "starlight/core/utils/Id.hh"
#include "starlight/core/memory/Memory.hh"

#include "starlight/renderer/Material.hh"

namespace sl {

class MaterialFactory
    : public ResourceFactory<Material>,
      public kc::core::Singleton<MaterialFactory> {
public:
    explicit MaterialFactory(const std::string& path);

    ResourceRef<Material> create(
      const std::string& name,
      const Material::Properties& properties = Material::Properties::createDefault()
    );
    ResourceRef<Material> create(
      const Material::Properties& properties = Material::Properties::createDefault()
    );
    ResourceRef<Material> load(
      const std::string& name, const FileSystem& fs = fileSystem
    );
    ResourceRef<Material> getDefault();

private:
    std::string m_materialsPath;
    ResourceRef<Material> m_defaultMaterial;
};

}  // namespace sl
