#include "Config.hh"

#include "starlight/core/Json.hh"
#include "starlight/core/Log.hh"

namespace sl {

void deserialize(const nlohmann::json& j, Config& out) {
    const auto& window = j.at("window");
    window.at("width").get_to(out.window.width);
    window.at("height").get_to(out.window.height);
    window.at("name").get_to(out.window.name);

    const auto& version = j.at("version");
    version.at("major").get_to(out.version.major);
    version.at("minor").get_to(out.version.minor);
    version.at("build").get_to(out.version.build);

    const auto& paths = j.at("paths");
    paths.at("textures").get_to(out.paths.textures);
    paths.at("shaders").get_to(out.paths.shaders);
    paths.at("materials").get_to(out.paths.materials);
    paths.at("fonts").get_to(out.paths.fonts);
}

std::optional<Config> Config::fromJson(
  const std::string& path, const FileSystem& fs
) {
    if (not fs.isFile(path)) {
        log::error("Config file '{}' does not exist", path);
        return {};
    }

    try {
        return nlohmann::json::parse(fs.readFile(path)).get<Config>();
    } catch (const nlohmann::json::parse_error& e) {
        log::error("Could not parse config file '{}' - {}", path, e.what());
    }
    return {};
}

}  // namespace sl
