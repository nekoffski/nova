// that header must be included first due to GLAD
#include "sl/platform/gpu/GraphicsContext.h"

#include <GLFW/glfw3.h>

namespace sl::platform::gpu::opengl {

class OpenGLGraphicsContext : public GraphicsContext {
public:
    explicit OpenGLGraphicsContext(void*);

    void init() override;
    void swapBuffers() override;
    void clearBuffers(unsigned) override;
    void setViewport(unsigned, unsigned) override;

private:
    GLFWwindow* m_windowHandle;
};

} // namespace sl::platform::gpu::opengl