#include "Core.hh"

#include "starlight/core/Log.hh"

namespace sl {

std::string toString(CullMode mode) {
    switch (mode) {
        case CullMode::none:
            return "none";
        case CullMode::front:
            return "front";
        case CullMode::back:
            return "back";
        case CullMode::frontAndBack:
            return "frontAndBack";
    }
    log::panic("Could not parse cull mode");
}

template <> CullMode fromString<CullMode>(std::string_view mode) {
    if (mode == "none")
        return CullMode::none;
    else if (mode == "front")
        return CullMode::front;
    else if (mode == "back")
        return CullMode::back;
    else if (mode == "frontAndBack")
        return CullMode::frontAndBack;
    log::panic("Could not parse cull mode: {}", mode);
}

std::string toString(PolygonMode polygonMode) {
    switch (polygonMode) {
        case PolygonMode::fill:
            return "fill";
        case PolygonMode::line:
            return "line";
        case PolygonMode::point:
            return "point";
    }
    log::panic("Could not parse polygon mode");
}

template <> PolygonMode fromString<PolygonMode>(std::string_view polygonName) {
    if (polygonName == "line")
        return PolygonMode::line;
    else if (polygonName == "fill")
        return PolygonMode::fill;
    else if (polygonName == "point")
        return PolygonMode::point;
    log::panic("Could not parse polygon mode: {}", polygonName);
}

}  // namespace sl
