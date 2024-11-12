#pragma once

#include <vector>

#include "starlight/core/Core.hh"
#include "starlight/core/memory/Memory.hh"
#include "starlight/core/window/Window.hh"
#include "starlight/core/Context.hh"
#include "starlight/core/FileSystem.hh"

#include "starlight/renderer/camera/EulerCamera.hh"
#include "starlight/renderer/camera/FirstPersonCamera.hh"
#include "starlight/renderer/RendererFrontend.hh"
#include "starlight/renderer/views/RenderView.hh"

#include "starlight/scene/Scene.hh"
// #include "starlight/scene/SceneSerializer.hh"
// #include "starlight/scene/SceneDeserializer.hh"

#include "ui/UI.hh"

class Application {
public:
    explicit Application(int argc, char** argv);

    int run();

private:
    void setupEventHandlers();
    void calculateViewport();

    sl::Rect2<sl::u32> m_viewport;

    bool m_isRunning;
    bool m_update;

    sl::Context m_context;
    sl::Window& m_window;
    sl::RendererFrontend m_renderer;

    UI m_ui;
    Logger* m_logger;

    sl::UniqPtr<sl::EulerCamera> m_eulerCamera;
    sl::UniqPtr<sl::FirstPersonCamera> m_firstPersonCamera;
    sl::Camera* m_activeCamera;

    sl::FileSystem m_fileSystem;

    sl::Scene m_scene;
    sl::SceneSerializer m_sceneSerializer;
    sl::SceneDeserializer m_sceneDeserializer;

    std::vector<sl::RenderView*> m_views;
    sl::FrameStatistics m_frameStatistics;
};
