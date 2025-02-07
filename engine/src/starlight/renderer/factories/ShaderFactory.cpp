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

static void sortMembers(Shader::Properties& props) {
    std::ranges::sort(props.inputAttributes, [](auto& lhs, auto& rhs) -> bool {
        return lhs.location < rhs.location;
    });

    std::ranges::sort(props.uniforms, [](auto& lhs, auto& rhs) -> bool {
        if (lhs.scope == rhs.scope) {
            if (lhs.type == Shader::DataType::sampler) return false;
            if (rhs.type == Shader::DataType::sampler) return true;

            return lhs.offset < rhs.offset;
        }
        return lhs.scope < rhs.scope;
    });

    props.uniforms.erase(
      std::unique(
        props.uniforms.begin(), props.uniforms.end(),
        [](auto& lhs, auto& rhs) -> bool {
            return lhs.name == rhs.name && lhs.scope == rhs.scope;
        }
      ),
      props.uniforms.end()
    );
}

static void logShaderProperties(const Shader::Properties& props) {
    log::debug("Shader parsed");
    log::debug("\tStages:");
    for (u32 i = 0u; auto& stage : props.stages)
        log::debug("\t\t{:02}. {}", i++, stage);

    log::debug("\tInput attributes:");
    for (u32 i = 0u; auto& attribute : props.inputAttributes)
        log::debug("\t\t{:02}. {}", i++, attribute);

    log::debug("\tUnforms:");
    for (u32 i = 0u; auto& uniform : props.uniforms)
        log::debug("\t\t{:02}. {}", i++, uniform);
}

std::optional<Shader::Properties> parseShader(
  const std::string& basePath, const FileSystem& fs
) {
    Shader::Properties props;
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
                props.stages.push_back(Shader::Stage{
                  .fullPath   = stagePath,
                  .sourceCode = source,
                  .type       = type,
                });

                std::ranges::move(
                  output->attributes, std::back_inserter(props.inputAttributes)
                );
                std::ranges::move(
                  output->uniforms, std::back_inserter(props.uniforms)
                );
            }
        }
    }

    if (props.stages.empty()) {
        log::warn("Could not parse shader, no stages found");
        return {};
    }

    sortMembers(props);
    logShaderProperties(props);

    return props;
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
