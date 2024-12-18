#pragma once

#include <typeindex>

namespace sle {

struct ComponentUIBase {
    virtual ~ComponentUIBase() = default;

    virtual bool renderSceneNode(void* component) = 0;
    virtual std::type_index getTypeIndex() const  = 0;
};

template <typename T> class ComponentUI : public ComponentUIBase {
public:
    virtual ~ComponentUI() override = default;

    std::type_index getTypeIndex() const override { return typeid(T); }

    virtual bool renderSceneNode(T& component) = 0;

private:
    bool renderSceneNode(void* component) override {
        return renderSceneNode(*static_cast<T*>(component));
    }
};

}  // namespace sle
