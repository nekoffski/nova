#pragma once

#include <typeindex>

#include "starlight/core/Json.hh"
#include "starlight/core/Log.hh"

namespace sl {

struct ComponentSerializerBase {
    virtual ~ComponentSerializerBase()                = default;
    virtual kc::json::Node serialize(void* ptr) const = 0;
    virtual std::string getName() const               = 0;
    virtual std::type_index getTypeIndex() const      = 0;
};

template <typename Component>
class ComponentSerializer : public ComponentSerializerBase {
public:
    virtual kc::json::Node serialize(Component& component) const = 0;

    std::type_index getTypeIndex() const override { return typeid(Component); }

private:
    // TODO: find better way
    kc::json::Node serialize(void* ptr) const override {
        auto component = static_cast<Component*>(ptr);
        // log::expect(
        //   component, "Invalid component serializer: {}", typeid(Component).name()
        // );
        return serialize(*component);
    }
};

}  // namespace sl
