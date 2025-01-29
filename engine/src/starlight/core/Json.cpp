#include "Json.hh"

namespace sl::json {

bool hasField(const nlohmann::json& j, std::string_view field) {
    return j.count(field) > 0;
}

}  // namespace sl::json