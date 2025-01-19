#pragma once

#include <memory>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <fmt/core.h>

#include "starlight/renderer/camera/Camera.hh"

#include "starlight/core/math/Core.hh"
#include "starlight/renderer/gpu/UIRenderer.hh"

namespace sl::ui {

constexpr Vec2<f32> parentWidth = { -1.0f, 0.0f };

using Callback = std::function<void()>;  // TODO: measure if it causes bottlenecks ->
                                         // if yes implement lightweight wrapper

bool checkbox(const std::string& label, bool& value);

void indent(Callback&& callback);

void pushTextColor(const Vec3<f32>& color);
void popTextColor(int count = 1);
void withColor(const Vec3<f32>& color, Callback&& callback);

void namedScope(const std::string& name, Callback&& callback);

void group(Callback&& callback);

void treeNode(
  const std::string& name, std::function<void()>&& callback,
  i32 flags = ImGuiTreeNodeFlags_None
);

bool wasItemClicked();

template <typename... Args> bool text(const std::string& fmt, Args&&... args) {
    ImGui::Text(
      "%s", fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...).c_str()
    );
    return ImGui::IsItemClicked();
}

bool slider(
  const std::string& name, Vec3<f32>& data, const StepInterval<float>& range
);

bool button(const std::string& text, const Vec2<f32>& size = { 0.0f, 0.0f });

void closeCurrentPopup();
void sameLine();
void lineBreak();
void separator();

float getFrameHeight();

// TODO: add concepts
void combo(
  const std::string& name, const std::string& preview, auto& container,
  auto&& onSelect
) {
    sl::ui::text("{}: ", name);
    if (ImGui::BeginCombo(
          fmt::format("##{}-combo", name).c_str(), preview.c_str()
        )) {
        for (auto& option : container) {
            bool selected = option.getName() == preview;
            if (ImGui::Selectable(option.getName().c_str(), selected))
                onSelect(option);
            if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
}

}  // namespace sl::ui
