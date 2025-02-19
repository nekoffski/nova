#include "ShaderFactory.hh"

#include "shader/SPIRVParser.hh"

#include <ranges>

namespace sl {

static const std::vector<std::pair<std::string, Shader::Stage::Type>>
  acceptedExtensions = {
      { "frag", Shader::Stage::Type::fragment },
      { "vert", Shader::Stage::Type::vertex   },
      { "geom", Shader::Stage::Type::geometry },
      { "comp", Shader::Stage::Type::compute  },
};

static void removeDuplicates(std::vector<Shader::Uniform>& uniforms) {
    std::ranges::sort(uniforms, [](auto& lhs, auto& rhs) -> bool {
        if (lhs.scope == rhs.scope) {
            if (lhs.binding == rhs.binding) return lhs.offset < rhs.offset;
            return lhs.binding < rhs.binding;
        }
        return lhs.scope < rhs.scope;
    });

    uniforms.erase(
      std::unique(
        uniforms.begin(), uniforms.end(),
        [](auto& lhs, auto& rhs) -> bool {
            return lhs.name == rhs.name && lhs.scope == rhs.scope;
        }
      ),
      uniforms.end()
    );
}

std::optional<Shader::Properties> parseShader(
  const std::string& basePath, const FileSystem& fs
) {
    std::vector<Shader::Stage> stages;
    std::vector<Shader::Uniform> uniforms;
    std::vector<Shader::InputAttribute> attributes;

    stages.reserve(Shader::maxStages);

    log::debug("Processing shader program: '{}'", basePath);

    for (const auto& [extension, type] : acceptedExtensions) {
        const auto stagePath = fmt::format("{}.{}.spv", basePath, extension);
        if (fs.isFile(stagePath)) {
            log::debug(
              "Found {} stage for '{}' shader, will try to process", type, basePath
            );
            const auto source = fs.readFile(stagePath);
            if (auto output = SPIRVParser{ source }.process(type); not output) {
                log::warn("Could not parse shader stage: {}", stagePath);
                return {};
            } else {
                stages.push_back(Shader::Stage{
                  .fullPath   = stagePath,
                  .sourceCode = source,
                  .type       = type,
                });

                std::ranges::move(
                  output->attributes, std::back_inserter(attributes)
                );
                std::ranges::move(output->uniforms, std::back_inserter(uniforms));
            }
        }
    }

    if (stages.empty()) {
        log::warn("Could not parse shader, no stages found");
        return {};
    }

    removeDuplicates(uniforms);

    Shader::Properties properties{
        .stages = stages,
        .layout = Shader::DataLayout{ attributes, uniforms },
    };

    log::debug("Processed shader:");
    logObject(properties);

    return properties;
}

ResourceRef<Shader> ShaderFactory::load(
  const std::string& name, const FileSystem& fs
) {
    const auto basePath = fmt::format("{}/{}", m_shadersPath, name);

    if (auto properties = parseShader(basePath, fs); properties)
        return store(name, m_device.createShader(*properties));

    log::warn("Could not parse shader properties");
    return {};
}

ShaderFactory::ShaderFactory(const std::string& path, Device& device) :
    ResourceFactory("Shader"), m_shadersPath(path), m_device(device) {}

}  // namespace sl
