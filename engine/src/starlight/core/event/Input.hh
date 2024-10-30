#pragma once

#include <string>

#include <fmt/core.h>

#include <kc/core/Macros.h>
#include <kc/core/String.h>

#include "starlight/core/window/Keys.hh"

namespace sl {

enum class KeyAction : unsigned char { press, repeat, release, unknown };
enum class MouseAction : unsigned char { press, release, unknown };

struct KeyEvent {
    KeyAction action;
    int key;
};

struct MouseEvent {
    MouseAction action;
    int button;
};

struct ScrollEvent {
    float offset;
};

}  // namespace sl

DEFINE_TO_STRING(sl::KeyAction, action) {
    switch (action) {
        case sl::KeyAction::press:
            return "KeyPress";

        case sl::KeyAction::repeat:
            return "KeyRepeat";

        case sl::KeyAction::release:
            return "KeyRelease";

        default:
            return "KeyUnknownAction";
    }
}

DEFINE_TO_STRING(sl::MouseAction, action) {
    switch (action) {
        case sl::MouseAction::press:
            return "KeyPress";

        case sl::MouseAction::release:
            return "KeyRelease";
        default:
            return "KeyUnknownAction";
    }
}

DEFINE_TO_STRING(sl::KeyEvent, event) {
    return fmt::format("KeyEvent[{}/{}]", event.key, event.action);
}

DEFINE_TO_STRING(sl::MouseEvent, event) {
    return fmt::format("MouseEvent[{}/{}]", event.button, event.action);
}

DEFINE_TO_STRING(sl::ScrollEvent, event) {
    return fmt::format("ScrollEvent[{}]", event.offset);
}