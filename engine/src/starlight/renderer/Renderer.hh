#pragma once

#include <vector>

#include "starlight/core/memory/Memory.hh"
#include "starlight/core/Core.hh"
#include "starlight/core/Context.hh"

#include "gpu/Device.hh"
#include "gpu/Swapchain.hh"
#include "gpu/Sync.hh"
#include "gpu/CommandBuffer.hh"
#include "gpu/Buffer.hh"

#include "factories/ShaderFactory.hh"
#include "factories/TextureFactory.hh"
#include "factories/SkyboxFactory.hh"
#include "factories/MeshFactory.hh"
#include "factories/MaterialFactory.hh"

namespace sl {

class Renderer {
public:
    explicit Renderer(Context& context);

    Context& getContext();
    Window& getWindow();
    Device& getDevice();
    Swapchain& getSwapchain();
    Buffer& getVertexBuffer();
    Buffer& getIndexBuffer();

    template <typename Callback>
    requires Callable<Callback, void, CommandBuffer&, u8>
    void renderFrame(Callback&& callback) {
        if (auto imageIndex = beginFrame(); imageIndex) [[likely]] {
            callback(*m_commandBuffers[*imageIndex], *imageIndex);
            endFrame(*imageIndex);
        }
    }

private:
    void createSyncPrimitives();
    void createBuffers();

    Fence* getImageFence(u32 imageIndex);

    std::optional<u8> beginFrame();
    void endFrame(u32 imageIndex);

    Context& m_context;
    Window& m_window;
    Config m_config;

    OwningPtr<Device> m_device;
    OwningPtr<Swapchain> m_swapchain;

    OwningPtr<Buffer> m_vertexBuffer;
    OwningPtr<Buffer> m_indexBuffer;

    u8 m_currentFrame;
    u8 m_maxFramesInFlight;

    std::vector<OwningPtr<CommandBuffer>> m_commandBuffers;
    std::vector<OwningPtr<Semaphore>> m_imageAvailableSemaphores;
    std::vector<OwningPtr<Semaphore>> m_queueCompleteSemaphores;
    std::vector<OwningPtr<Fence>> m_frameFences;
    std::vector<Fence*> m_imageFences;

    // factories
    ShaderFactory m_shaderFactory;
    TextureFactory m_textureFactory;
    MaterialFactory m_materialFactory;
    MeshFactory m_meshFactory;
    SkyboxFactory m_skyboxFactory;
};

}  // namespace sl
