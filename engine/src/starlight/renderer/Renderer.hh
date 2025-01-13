#pragma once

#include "starlight/core/Core.hh"
#include "starlight/core/Context.hh"
#include "starlight/renderer/gpu/Device.hh"

namespace sl {

class Renderer {
public:
    explicit Renderer(Context& context);

    Context& getContext();

private:
    Context& m_context;

    OwningPtr<Device> m_device;
};

}  // namespace sl
