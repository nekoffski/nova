#include "RenderView.hh"

#include "starlight/core/window/Window.hh"
#include "starlight/core/math/Core.hh"

namespace sl {

RenderPass::Properties RenderView::getDefaultRenderPassProperties() {
    static RenderPass::Properties props;

    props.rect = Rect2u32{
        Vec2<u32>{ 0u, 0u },
        Window::get().getFramebufferSize()
    };
    props.clearColor = Vec4<f32>{ 1.0f };
    props.clearFlags = RenderPass::ClearFlags::none;

    return props;
}

}  // namespace sl
