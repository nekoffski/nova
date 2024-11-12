#pragma once

#include <optional>

#include "starlight/core/Core.hh"
#include "starlight/core/math/Core.hh"
#include "starlight/renderer/camera/Camera.hh"
#include "starlight/ui/UI.hh"

enum class ResourceType { mesh = 0, texture, shader, material };

std::string resourceTypeToString(ResourceType type);

struct UIState {
    std::optional<sl::u64> selectedEntityId;
    std::optional<sl::u64> selectedResourceId;
    std::optional<ResourceType> selectedResourceType;
    std::unordered_map<sl::u64, std::unique_ptr<sl::ui::ImageHandle>> imageHandles;

    sl::Camera* camera;
    sl::Rect2<sl::u32>* viewport;

    bool showGrid  = true;
    float gridSize = 25.0f;

    sl::ui::ImageHandle* getOrCreateImageHandle(sl::Texture* texture);
};

static constexpr float panelWidthFactor = 0.15f;

static const sl::Vec3<sl::f32> selectedColor = { 0.1f, 0.7f, 0.1f };
static const sl::Vec3<sl::f32> defaultColor  = { 1.0f, 1.0f, 1.0f };
