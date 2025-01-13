#include "Renderer.hh"

namespace sl {

Renderer::Renderer(Context& context
) : m_context(context), m_device(Device::create(m_context)) {}

Context& Renderer::getContext() { return m_context; }

}  // namespace sl
