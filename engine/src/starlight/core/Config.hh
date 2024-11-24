#pragma once

#include <optional>
#include <string>

#include "starlight/core/Core.hh"
#include "FileSystem.hh"

namespace sl {

struct Config {
    static std::optional<Config> fromJson(
      const std::string& path, const FileSystem& fs = fileSystem
    );

    std::string loggerIdent;

    struct Window {
        u32 width;
        u32 height;
        std::string name;
    } window;

    struct Version {
        u32 major;
        u32 minor;
        u32 build;
    } version;

    struct Paths {
        std::string textures;
        std::string shaders;
        std::string materials;
    } paths;
};

}  // namespace sl