#pragma once

#include <starlight/renderer/Material.hh>

#include "Resources.hh"

namespace sle {

class MaterialUI {
public:
    explicit MaterialUI(Resources& resources);

    void render(sl::SharedPtr<sl::Material> material);

private:
    Resources& m_resources;
};

}  // namespace sle
