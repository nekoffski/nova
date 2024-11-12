#include "Core.hh"

std::string resourceTypeToString(ResourceType type) {
    switch (type) {
        case ResourceType::mesh:
            return "Mesh";
        case ResourceType::texture:
            return "Texture";
        case ResourceType::shader:
            return "Shader";
    }
    return "Unknown";
}

sl::ui::ImageHandle* UIState::getOrCreateImageHandle(sl::Texture* texture) {
    auto id     = texture->getId();
    auto record = imageHandles.find(id);

    if (record != imageHandles.end()) return record->second.get();

    imageHandles[id] = sl::ui::ImageHandle::createHandle(texture);
    return imageHandles.at(id).get();
}
