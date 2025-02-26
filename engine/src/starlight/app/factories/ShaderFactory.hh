#pragma once

#include "starlight/core/Factory.hh"
#include "starlight/core/FileSystem.hh"
#include "starlight/renderer/gpu/Shader.hh"

namespace sl {

class ShaderFactory : public Factory<ShaderFactory, Shader> {
public:
    SharedPtr<Shader> load(
      const std::string& name, const FileSystem& fs = FileSystem::getDefault()
    );
};

}  // namespace sl
