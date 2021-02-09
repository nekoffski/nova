#pragma once

#include <memory>

#include "sl/asset/AssetLoader.hpp"
#include "sl/graphics/Shader.h"

namespace sl::utils::globals {

struct Shaders {
    explicit Shaders() {
        defaultCubemapShader = asset::AssetLoader::loadLocalPath<graphics::Shader>("/cubemap.vert", "/cubemap.frag");
		defaultModelShader = asset::AssetLoader::loadLocalPath<graphics::Shader>("/t.vert", "/t.frag");
	}

    std::shared_ptr<graphics::Shader> defaultCubemapShader;
	std::shared_ptr<graphics::Shader> defaultModelShader;
};
}