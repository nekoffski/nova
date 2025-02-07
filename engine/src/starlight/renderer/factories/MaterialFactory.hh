#pragma once

#include <string>

#include "starlight/core/FileSystem.hh"
#include "starlight/core/Resource.hh"
#include "starlight/core/Id.hh"
#include "starlight/core/memory/Memory.hh"
#include "starlight/core/Singleton.hh"

#include "starlight/renderer/Material.hh"

namespace sl {

class MaterialFactory
    : public ResourceFactory<Material>,
      public Singleton<MaterialFactory> {
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
      const std::string& name, const FileSystem& fs = FileSystem::getDefault()
    );
    ResourceRef<Material> getDefault();

private:
    std::string m_materialsPath;
    ResourceRef<Material> m_defaultMaterial;
};

}  // namespace sl
