#pragma once

#include "sl/platform/shader/ShaderCompilerImpl.h"

namespace sl::platform::shader {
class Shader;
}

namespace sl::platform::shader::opengl {

class OpenGLShader;

class OpenGLShaderCompilerImpl : public ShaderCompilerImpl {
public:
    void compile(std::shared_ptr<Shader>& shader) override;

private:
    void compileImpl(std::shared_ptr<OpenGLShader>);
    unsigned int compileShader(const std::string&, unsigned);

    inline static constexpr int infoBufferSize{ 1024 };
};
}