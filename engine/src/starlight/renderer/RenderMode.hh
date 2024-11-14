#pragma once

#include <kc/core/String.h>

#include "starlight/core/Core.hh"

namespace sl {

enum class RenderMode : u8 { standard = 0, normals = 1, lights = 2 };

}

DEFINE_TO_STRING(sl::RenderMode, mode) {
    switch (mode) {
        case sl::RenderMode::standard:
            return "RenderMode[standard]";
        case sl::RenderMode::normals:
            return "RenderMode[normals]";
        case sl::RenderMode::lights:
            return "RenderMode[lights]";
    }
    return "RenderMode[unknown]";
}
