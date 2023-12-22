#pragma once

#include <array>
#include <span>
#include <functional>

#include "starlight/core/math/Glm.h"
#include "starlight/core/math/Vertex.h"
#include "starlight/core/math/Vertex.h"

#include "starlight/renderer/Geometry.h"
#include "starlight/renderer/Material.h"
#include "starlight/renderer/Texture.h"
#include "starlight/renderer/Shader.h"
#include "starlight/renderer/fwd.h"
#include "starlight/renderer/CommandBuffer.h"

#include "RendererBackendProxy.h"

namespace sl {

// TODO: change to objects or something
constexpr int builtinRenderPassWorld = 1;
constexpr int builtinRenderPassUI    = 2;

struct RendererBackend {
    virtual ~RendererBackend() = default;

    virtual bool beginFrame(float deltaTime) = 0;
    virtual bool endFrame(float deltaTime)   = 0;

    virtual void drawGeometry(const Geometry& geometry) = 0;

    virtual void onViewportResize(uint32_t width, uint32_t height) = 0;

    virtual RendererBackendProxy* getProxy()  = 0;
    virtual ResourcePools* getResourcePools() = 0;
};

}  // namespace sl