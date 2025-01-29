#include "ShaderFactory.hh"

#include "starlight/core/Json.hh"

#include "TextureFactory.hh"

namespace sl {

static std::optional<std::string> getShaderSource(
  std::string_view shadersPath, std::string_view name, const FileSystem& fs
) {
    const auto fullPath = fmt::format("{}/{}", shadersPath, name);

    if (not fs.isFile(fullPath)) {
        log::warn("Could not find shader file '{}'", fullPath);
        return {};
    }
    return fs.readFile(fullPath);
}

static std::vector<Shader::Stage> processStages(
  const nlohmann::json& root, std::string_view shadersPath, const FileSystem& fs
) {
    std::vector<Shader::Stage> stages;
    stages.reserve(root.size());

    for (const auto& stage : root) {
        const auto file      = stage.at("file").get<std::string>();
        const auto stageName = stage.at("stage").get<std::string>();

        auto source = getShaderSource(shadersPath, file, fs);
        log::expect(source.has_value(), "Could not find source file for: {}", file);

        stages.emplace_back(Shader::Stage::typeFromString(stageName), *source);
    }
    return stages;
}

static std::vector<Shader::Attribute> processAttributes(const nlohmann::json& root) {
    std::vector<Shader::Attribute> attributes;
    attributes.reserve(root.size());

    for (auto& attribute : root) {
        const auto type =
          Shader::Attribute::typeFromString(attribute.at("type").get<std::string>());
        const auto size = Shader::Attribute::getTypeSize(type);
        const auto name = attribute.at("name").get<std::string>();

        attributes.emplace_back(name, type, size);
    }

    return attributes;
};

static std::vector<Shader::Uniform::Properties> processUniforms(
  const nlohmann::json& root
) {
    std::vector<Shader::Uniform::Properties> uniforms;
    uniforms.reserve(root.size());

    static auto getSize =
      [](const nlohmann::json& uniform, Shader::Uniform::Type type) -> u64 {
        if (type == Shader::Uniform::Type::custom) {
            auto size         = uniform.at("size").get<u32>();
            auto elementCount = uniform.at("elements").get<unsigned int>();

            return size * elementCount;
        } else {
            return Shader::Uniform::getTypeSize(type);
        }
    };

    for (auto& uniform : root) {
        const auto type =
          Shader::Uniform::typeFromString(uniform.at("type").get<std::string>());

        const auto size  = getSize(uniform, type);
        const auto name  = uniform.at("name").get<std::string>();
        const auto scope = uniform.at("scope").get<std::string>();

        uniforms.emplace_back(name, size, 0, type, Shader::scopeFromString(scope));
    }

    return uniforms;
};

static std::optional<Shader::Properties> loadPropertiesFromFile(
  std::string_view name, Texture* defaultTexture, std::string_view shadersPath,
  const FileSystem& fs
) {
    const auto fullPath = fmt::format("{}/{}.json", shadersPath, name);

    log::trace("Loading shader config file: {}", fullPath);

    if (not fs.isFile(fullPath)) {
        log::error("Could not find file: '{}'", fullPath);
        return {};
    }

    try {
        auto root = nlohmann::json::parse(fs.readFile(fullPath));
        return Shader::Properties{
            .useInstances      = root.at("use-instances").get<bool>(),
            .useLocals         = root.at("use-local").get<bool>(),
            .attributes        = processAttributes(root.at("attributes")),
            .stages            = processStages(root.at("stages"), shadersPath, fs),
            .uniformProperties = processUniforms(root.at("uniforms")),
            .defaultTexture    = defaultTexture,
            .cullMode =
              cullModeFromString(json::getOr<std::string>(root, "cullMode", "back")),
            .polygonMode = polygonModeFromString(
              json::getOr<std::string>(root, "polygonMode", "fill")
            )
        };
    } catch (nlohmann::json::parse_error& e) {
        log::error("Could not parse shader '{}' file: {}", name, e.what());
    }
    return {};
}

ShaderFactory::ShaderFactory(const std::string& path, Device& device) :
    ResourceFactory("Shader"), m_shadersPath(path), m_device(device) {}

ResourceRef<Shader> ShaderFactory::load(
  const std::string& name, const FileSystem& fs
) {
    if (auto resource = find(name); resource) {
        log::trace("Shader '{}' found, returning from cache", name);
        return resource;
    }

    const auto properties = loadPropertiesFromFile(
      name, TextureFactory::get().getDefaultDiffuseMap(), m_shadersPath, fs
    );

    if (not properties) {
        log::warn("Could not load properties from '{}/{}'", m_shadersPath, name);
        return nullptr;
    }

    return store(name, m_device.createShader(*properties));
}

}  // namespace sl