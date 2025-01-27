#include "Config.hh"

#include "starlight/core/utils/Json.hh"
#include "starlight/core/Log.hh"

namespace sl {

std::optional<Config> Config::fromJson(
  const std::string& path, const FileSystem& fs
) {
    if (not fs.isFile(path)) {
        log::error("Config file '{}' does not exist", path);
        return {};
    }

    try {
        auto root = kc::json::loadJson(fs.readFile(path));

        auto window  = root["window"];
        auto version = root["version"];
        auto paths   = root["paths"];

        return Config{
            .window = { 
                .width  = getField<u32>(window, "width"),
                .height = getField<u32>(window, "height"),
                .name   = getField<std::string>(window, "name"), 
            },
            .version = {
                .major = getField<u32>(version, "major"),
                .minor = getField<u32>(version, "minor"),
                .build = getField<u32>(version, "build"),
            },
            .paths = {
                .textures = getField<std::string>(paths, "textures"),
                .shaders = getField<std::string>(paths, "shaders"),
                .materials = getField<std::string>(paths, "materials"),
                .fonts = getField<std::string>(paths, "fonts")
            }
        };
    } catch (const kc::json::JsonError& e) {
        log::error("Could not parse config file '{}' - {}", path, e.asString());
    }
    return {};
}

}  // namespace sl
