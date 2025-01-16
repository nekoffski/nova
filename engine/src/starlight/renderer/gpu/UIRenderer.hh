// #pragma once

// #include <unordered_map>
// #include <string>
// #include <span>

// #include "starlight/core/memory/Memory.hh"
// #include "starlight/core/Core.hh"
// #include "starlight/renderer/fwd.hh"

// // TODO: hide
// #include <imgui.h>

// namespace sl {

// struct Font {
//     struct SubfontProperties {
//         std::string path;
//         u16 minIndex;
//         u16 maxIndex;
//     };

//     struct Properties {
//         std::string name;
//         std::string path;
//         u16 size;
//         std::vector<SubfontProperties> subfonts;
//     };

//     ImFont* handle;
//     std::vector<std::array<ImWchar, 3>> mergedFontsRanges;
// };

// class UIRenderer {
// public:
//     template <typename C, typename... Args>
//     requires Callable<C, void, Args...>
//     void render(CommandBuffer& commandBuffer, C&& callback, Args&&... args) {
//         begin(commandBuffer);
//         ImGui::PushFont(m_fonts["main-font"].handle);  // temporary
//         callback(std::forward<Args>(args)...);
//         ImGui::PopFont();
//         end(commandBuffer);
//     }

//     static OwningPtr<UIRenderer> create(RendererBackend&, RenderPass& renderPass);

//     virtual ~UIRenderer() = default;

//     // TODO: pass params
//     void setStyle();
//     Font* addFont(const Font::Properties& props);

//     virtual void reloadFontTextures() = 0;

// protected:
//     virtual void begin(CommandBuffer& commandBuffer) = 0;
//     virtual void end(CommandBuffer& commandBuffer)   = 0;

// private:
//     std::unordered_map<std::string, Font> m_fonts;
// };

// }  // namespace sl
