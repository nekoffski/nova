#pragma once

#include "starlight/core/Singleton.hh"

#include "starlight/core/math/Core.hh"
#include "WindowImpl.hh"

namespace sl {

class Window : public Singleton<Window> {
public:
    explicit Window(WindowImpl& window);

    Vec2<u32> getSize() const;
    Vec2<u32> getFramebufferSize() const;

    u32 getFramebufferWidth() const;
    u32 getFramebufferHeight() const;
    u32 getWidth() const;
    u32 getHeight() const;

    void* getHandle();

private:
    void setCallbacks();

    WindowImpl& m_windowImpl;
};

}  // namespace sl
