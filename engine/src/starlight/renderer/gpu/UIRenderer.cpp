#include "UIRenderer.hh"

#include "starlight/core/Log.hh"

#include <imgui.h>

#ifdef SL_USE_VK
#include "starlight/renderer/gpu/vulkan/VKRendererBackend.hh"
#include "starlight/renderer/gpu/vulkan/VKUIRenderer.hh"
#endif

namespace sl {

OwningPtr<UIRenderer> UIRenderer::create(
  RendererBackend& renderer, RenderPass& renderPass
) {
#ifdef SL_USE_VK
    // auto& vkRenderer = static_cast<vk::VKRendererBackend&>(renderer);

    // return createOwningPtr<vk::VKUIRenderer>(
    //   vkRenderer.getContext(), vkRenderer.getPhysicalDevice(),
    //   vkRenderer.getLogicalDevice(), vkRenderer.getWindow(), renderPass
    // );
#else
    FATAL_ERROR("Could not find renderer backend implementation");
#endif
}

void UIRenderer::setStyle() {
    // TODO: move it somewhere closer to the frontend
    ImGuiStyle& style = ImGui::GetStyle();

    // light style from Pacôme Danhiez (user itamago)
    // https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
    style.Alpha                    = 0.99f;
    style.WindowRounding           = 0.0f;
    style.FrameRounding            = 0.0f;
    style.WindowMenuButtonPosition = ImGuiDir_None;

    auto& colors              = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

    // Headers
    colors[ImGuiCol_Header]        = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
    colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
    colors[ImGuiCol_HeaderActive]  = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

    // Buttons
    colors[ImGuiCol_Button]        = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
    colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
    colors[ImGuiCol_ButtonActive]  = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

    // Frame BG
    colors[ImGuiCol_FrameBg]        = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
    colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
    colors[ImGuiCol_FrameBgActive]  = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

    // Tabs
    colors[ImGuiCol_Tab]                = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    colors[ImGuiCol_TabHovered]         = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
    colors[ImGuiCol_TabActive]          = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
    colors[ImGuiCol_TabUnfocused]       = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

    // Title
    colors[ImGuiCol_TitleBg]          = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    colors[ImGuiCol_TitleBgActive]    = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
}

Font* UIRenderer::addFont(const Font::Properties& props) {
    auto [iterator, inserted] = m_fonts.insert({ props.name, Font{} });
    ASSERT(inserted, "Font '{}' already exists", props.name);

    auto& font      = iterator->second;
    auto imguiFonts = ImGui::GetIO().Fonts;
    LOG_INFO("Loading font: '{}' - '{}'", props.name, props.path);

    font.handle = imguiFonts->AddFontFromFileTTF(props.path.c_str(), props.size);
    ASSERT(font.handle, "Could not load font '{}'", props.name);

    if (const auto subfontSize = props.subfonts.size(); subfontSize > 0) {
        font.mergedFontsRanges.reserve(subfontSize);

        for (const auto& subfont : props.subfonts) {
            font.mergedFontsRanges.push_back(
              { subfont.minIndex, subfont.maxIndex, 0 }
            );

            LOG_INFO("Loading subfont: '{}' - '{}'", props.name, subfont.path);

            ImFontConfig config;
            config.MergeMode        = true;
            config.GlyphMinAdvanceX = 12.0f;
            config.GlyphMaxAdvanceX = 15.0f;
            config.GlyphOffset.y    = 1.0f;

            ASSERT(
              imguiFonts->AddFontFromFileTTF(
                subfont.path.c_str(), props.size - 2.0f, &config,
                font.mergedFontsRanges.back().data()
              ),
              "Could not merge font: '{}'", subfont.path
            );
        }
    }
    reloadFontTextures();
    return &font;
}

}  // namespace sl
