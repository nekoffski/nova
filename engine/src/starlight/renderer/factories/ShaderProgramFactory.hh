#pragma once

#include "starlight/core/Resource.hh"
#include "starlight/core/Singleton.hh"
#include "starlight/core/FileSystem.hh"
#include "starlight/renderer/gpu/ShaderProgram.hh"

namespace sl {

class ShaderProgramFactory
    : public ResourceFactory<ShaderProgram>,
      public Singleton<ShaderProgramFactory> {
public:
    explicit ShaderProgramFactory(const std::string& path);

    ResourceRef<ShaderProgram> load(
      const std::string& name, const FileSystem& fs = FileSystem::getDefault()
    );

private:
    const std::string m_shaderProgramsPath;
};

}  // namespace sl