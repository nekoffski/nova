#pragma once

#include "starlight/core/Resource.hh"
#include "starlight/core/Singleton.hh"
#include "starlight/renderer/gpu/Shader.hh"
#include "starlight/renderer/gpu/Device.hh"

namespace sl {

class ShaderFactory
    : public ResourceFactory<Shader>,
      public Singleton<ShaderFactory> {
public:
    explicit ShaderFactory(const std::string& path, Device& device);

    ResourceRef<Shader> load(
      const std::string& name, const FileSystem& fs = fileSystem
    );

private:
    const std::string m_shadersPath;
    Device& m_device;
};

}  // namespace sl