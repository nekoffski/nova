#pragma once

#include "starlight/core/math/Core.hh"
#include "starlight/core/Utils.hh"
#include "Keys.hh"

namespace sl {

enum class KeyAction : unsigned char { press, repeat, release, unknown };
enum class MouseAction : unsigned char { press, release, unknown };

struct QuitEvent {
    std::string reason;
};

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

struct WindowResized {
    Vec2<u32> size;
};

inline std::string toString(const WindowResized& event) {
    return fmt::format("WindowResized[{}/{}]", event.size.w, event.size.h);
}

inline std::string toString(const KeyAction& action) {
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

inline std::string toString(const MouseAction& action) {
    switch (action) {
        case sl::MouseAction::press:
            return "KeyPress";

        case sl::MouseAction::release:
            return "KeyRelease";
        default:
            return "KeyUnknownAction";
    }
}

inline std::string toString(const KeyEvent& event) {
    return fmt::format("KeyEvent[{}/{}]", event.key, event.action);
}

inline std::string toString(const MouseEvent& event) {
    return fmt::format("MouseEvent[{}/{}]", event.button, event.action);
}

inline std::string toString(const ScrollEvent& event) {
    return fmt::format("ScrollEvent[{}]", event.offset);
}

}  // namespace sl
