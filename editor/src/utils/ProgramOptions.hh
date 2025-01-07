
#include <string>
#include <optional>

namespace sle {

struct ProgramOptions {
    static std::optional<ProgramOptions> parse(int argc, char** argv);

    std::string configPath;
    std::optional<std::string> scene;
};

}  // namespace sle
