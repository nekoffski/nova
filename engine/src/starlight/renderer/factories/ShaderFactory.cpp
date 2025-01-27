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
  const kc::json::Node& root, std::string_view shadersPath, const FileSystem& fs
) {
    std::vector<Shader::Stage> stages;
    stages.reserve(root.size());

    for (auto& stage : root) {
        const auto file      = getField<std::string>(stage, "file");
        const auto stageName = getField<std::string>(stage, "stage");

        auto source = getShaderSource(shadersPath, file, fs);
        log::expect(source.has_value(), "Could not find source file for: {}", file);

        stages.emplace_back(Shader::Stage::typeFromString(stageName), *source);
    }
    return stages;
}

static std::vector<Shader::Attribute> processAttributes(const kc::json::Node& root) {
    std::vector<Shader::Attribute> attributes;
    attributes.reserve(root.size());

    for (auto& attribute : root) {
        const auto type = Shader::Attribute::typeFromString(
          getField<std::string>(attribute, "type")
        );
        const auto size = Shader::Attribute::getTypeSize(type);
        const auto name = getField<std::string>(attribute, "name");

        attributes.emplace_back(name, type, size);
    }

    return attributes;
};

static std::vector<Shader::Uniform::Properties> processUniforms(
  const kc::json::Node& root
) {
    std::vector<Shader::Uniform::Properties> uniforms;
    uniforms.reserve(root.size());

    static auto getSize =
      [](const kc::json::Node& uniform, Shader::Uniform::Type type) -> u64 {
        if (type == Shader::Uniform::Type::custom) {
            auto size         = getField<unsigned int>(uniform, "size");
            auto elementCount = getField<unsigned int>(uniform, "elements");

            return size * elementCount;
        } else {
            return Shader::Uniform::getTypeSize(type);
        }
    };

    for (auto& uniform : root) {
        const auto type =
          Shader::Uniform::typeFromString(getField<std::string>(uniform, "type"));

        const auto size  = getSize(uniform, type);
        const auto name  = getField<std::string>(uniform, "name");
        const auto scope = getField<std::string>(uniform, "scope");

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
        auto root = kc::json::loadJson(fs.readFile(fullPath));
        return Shader::Properties{
            .useInstances = getField<bool>(root, "use-instances"),
            .useLocals    = getField<bool>(root, "use-local"),
            .attributes   = processAttributes(getArray(root, "attributes")),
            .stages       = processStages(getArray(root, "stages"), shadersPath, fs),
            .uniformProperties = processUniforms(getArray(root, "uniforms")),
            .defaultTexture    = defaultTexture,
            .cullMode =
              cullModeFromString(getFieldOr<std::string>(root, "cullMode", "back")),
            .polygonMode = polygonModeFromString(
              getFieldOr<std::string>(root, "polygonMode", "fill")
            )
        };
    } catch (kc::json::JsonError& e) {
        log::error("Could not parse shader '{}' file: {}", name, e.asString());
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