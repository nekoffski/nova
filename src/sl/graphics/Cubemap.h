#pragma once

#include <memory>
#include <string>

namespace sl::graphics {

using CubemapArgs = std::array<std::string, 6>;

class Cubemap {
public:
    struct Factory {
        virtual std::shared_ptr<Cubemap> create(const CubemapArgs&) = 0;
    };

    inline static std::unique_ptr<Factory> factory = nullptr;

    virtual ~Cubemap() = default;

    virtual void bind() = 0;
    virtual void unbind() = 0;
};
}