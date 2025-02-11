#include "Pipeline.hh"

#include "starlight/core/window/Window.hh"

namespace sl {

Pipeline::Properties Pipeline::Properties::createDefault() {
    const auto size = Window::get().getSize();

    static Vec2<u32> origin{ 0u, 0u };

    return Properties{
        .viewport         = { origin, size },
        .scissor          = { origin, size },
        .polygonMode      = PolygonMode::fill,
        .cullMode         = CullMode::back,
        .depthTestEnabled = true,
    };
}

}  // namespace sl
