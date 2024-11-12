#include "Application.hh"

#include "starlight/core/event/Quit.hh"
#include "starlight/core/event/WindowResized.hh"
#include "starlight/core/event/Input.hh"

#include "starlight/renderer/views/UIRenderView.hh"
#include "starlight/renderer/views/WorldRenderView.hh"
#include "starlight/renderer/views/SkyboxRenderView.hh"
#include "starlight/renderer/RenderPacket.hh"

#include "starlight/ui/UI.hh"
#include "starlight/ui/fonts/FontAwesome.hh"

#include "Events.hh"

Application::Application(int argc, char** argv) :
    m_isRunning(false), m_update(false), m_context("Starlight Editor"),
    m_window(m_context.getWindow()), m_renderer(m_context), m_activeCamera(nullptr),
    m_ui(
      m_window.getSize().w,
      m_window.getSize().h,  // TODO: no comment required
      m_renderer, &m_scene
    ),
    m_logger(m_ui.getLogger()), m_sceneSerializer(m_fileSystem),
    m_sceneDeserializer(m_fileSystem) {
    const auto framebufferSize = m_window.getFramebufferSize();

    m_eulerCamera = sl::createUniqPtr<sl::EulerCamera>(sl::EulerCamera::Properties{
      .target       = sl::Vec3<sl::f32>{ 0.0f },
      .radius       = 5.0f,
      .viewportSize = framebufferSize });
    m_firstPersonCamera =
      sl::createUniqPtr<sl::FirstPersonCamera>(sl::FirstPersonCamera::Properties{
        .position = sl::Vec3<sl::f32>{ 0.0f }, .viewportSize = framebufferSize });

    m_activeCamera = m_eulerCamera.get();

    calculateViewport();

    m_ui.getState()->camera   = m_activeCamera;
    m_ui.getState()->viewport = &m_viewport;
}

int Application::run() {
    setupEventHandlers();

    sl::Font::SubfontProperties icons{
        "/home/nek0/kapik/projects/starlight/assets/fonts/fa-solid-900.ttf",
        ICON_MIN_FA, ICON_MAX_FA
    };
    sl::Font::Properties font{
        .name = "main-font",
        .path =
          "/home/nek0/kapik/projects/starlight/assets/fonts/Roboto-Regular.ttf",
        .size     = 15,
        .subfonts = { icons }
    };

    // sl::UIRenderView::Properties uiProperties;
    // uiProperties.fonts = { font };

    // sl::UIRenderView uiView(m_activeCamera, uiProperties, [&]() { m_ui.render();
    // });

    auto materialShader = sl::ShaderManager::get().load("Builtin.Shader.Material");
    sl::WorldRenderView worldView{ m_activeCamera, materialShader };

    auto skyboxShader = sl::ShaderManager::get().load("Builtin.Shader.Skybox");
    auto skybox = sl::SkyboxManager::get().load("skybox2/skybox", *skyboxShader);

    ASSERT(skybox, "Could not load skybox");
    // sl::SkyboxRenderView skyboxView{ m_activeCamera, skyboxShader, skybox };

    LOG_INFO("Starlight Editor starting");

    m_isRunning = true;
    // m_renderer.init(m_views);

    while (m_isRunning && not m_ui.shouldExit()) {
        m_context.beginFrame([&](float deltaTime) {
            LOG_VAR(deltaTime);
            LOG_VAR(m_activeCamera->getPosition());

            auto renderPacket = m_scene.getRenderPacket();
            // renderPacket.viewport = m_viewport;

            m_renderer.renderFrame(deltaTime, renderPacket);

            m_activeCamera->update(deltaTime);
        });
    }

    return 0;
}

void Application::setupEventHandlers() {
    const auto onQuit = [&](sl::QuitEvent* event) {
        LOG_INFO("Received QuitEvent, reason: '{}'", event->reason);
        m_isRunning = false;
    };

    const auto onKey = [&](sl::KeyEvent* event) {
        if (event->action == sl::KeyAction::press) {
            const auto& key = event->key;

            if ((key == SL_KEY_ESCAPE)) m_isRunning = false;
            if (key == SL_KEY_0) m_renderer.setRenderMode(sl::RenderMode::lights);
            if (key == SL_KEY_9) m_renderer.setRenderMode(sl::RenderMode::normals);
            if (key == SL_KEY_8) m_renderer.setRenderMode(sl::RenderMode::standard);
            if (key == SL_KEY_6) sl::enableVariableLogging();
            if (key == SL_KEY_4) {
                m_activeCamera          = m_eulerCamera.get();
                m_ui.getState()->camera = m_activeCamera;
                m_window.showCursor();
                for (auto& view : m_views) view->setCamera(m_activeCamera);
            }
            if (key == SL_KEY_3) {
                m_activeCamera          = m_firstPersonCamera.get();
                m_ui.getState()->camera = m_activeCamera;
                m_window.hideCursor();
                for (auto& view : m_views) view->setCamera(m_activeCamera);
            }
            if (key == SL_KEY_U) m_update = !m_update;
        }
    };

    const auto onScroll = [&](sl::ScrollEvent* event) {
        m_eulerCamera->onScroll(event->offset);
    };

    const auto onWindowResized = [&](sl::WindowResized* event) {
        sl::Vec2<sl::u32> viewportSize{ event->width, event->height };
        m_renderer.onViewportResize(viewportSize);
        m_eulerCamera->onViewportResize(viewportSize);
        m_firstPersonCamera->onViewportResize(viewportSize);

        calculateViewport();
    };

    const auto onSceneLoaded = [&](SceneLoaded* event) {
        m_logger->info("Loading scene: {}", event->path);
        m_sceneDeserializer.deserialize(m_scene, event->path);
    };

    const auto onSceneSaved = [&](SceneSaved* event) {
        m_logger->info("Saving scene: {}", event->path);
        m_sceneSerializer.serialize(m_scene, event->path);
    };

    sl::EventManager::get()
      .on<sl::QuitEvent>(onQuit)
      .on<sl::KeyEvent>(onKey)
      .on<sl::WindowResized>(onWindowResized)
      .on<sl::ScrollEvent>(onScroll)
      .on<SceneLoaded>(onSceneLoaded)
      .on<SceneSaved>(onSceneSaved);
}

void Application::calculateViewport() {
    const auto size = m_window.getFramebufferSize();

    m_viewport.offset.x = panelWidthFactor * size.w;
    m_viewport.offset.y = 0.25f * size.h;
    m_viewport.size.w   = (1.0f - 2.0f * panelWidthFactor) * size.w;
    m_viewport.size.h   = (0.75f) * size.h;
}
