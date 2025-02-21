#pragma once

#include "starlight/core/Singleton.hh"
#include "starlight/core/Config.hh"
#include "starlight/core/math/Core.hh"
#include "starlight/core/memory/UniquePointer.hh"

#include "Events.hh"

namespace sl {

class Window : public Singleton<Window> {
public:
    using Key    = int;
    using Button = int;

    struct Impl {
        using OnKeyCallback          = void (*)(KeyAction, Key);
        using OnMouseCallback        = void (*)(MouseAction, Button);
        using OnScrollCallback       = void (*)(float);
        using OnWindowCloseCallback  = void (*)();
        using OnWindowResizeCallback = void (*)(uint32_t, uint32_t);

        virtual ~Impl() = default;

        virtual std::string_view getVendor() const = 0;

        virtual bool isKeyPressed(Key keyCode) const               = 0;
        virtual bool isMouseButtonPressed(Button buttonCode) const = 0;

        virtual void update()      = 0;
        virtual void swapBuffers() = 0;

        virtual void showCursor() = 0;
        virtual void hideCursor() = 0;

        virtual Vec2<u32> getFramebufferSize() const = 0;
        virtual Vec2<u32> getSize() const            = 0;
        virtual Vec2<f32> getMousePosition() const   = 0;

        virtual void onKeyCallback(OnKeyCallback)                   = 0;
        virtual void onMouseCallback(OnMouseCallback)               = 0;
        virtual void onScrollCallback(OnScrollCallback)             = 0;
        virtual void onWindowCloseCallback(OnWindowCloseCallback)   = 0;
        virtual void onWindowResizeCallback(OnWindowResizeCallback) = 0;

        virtual void* getHandle() = 0;

        static UniquePointer<Impl> create();
    };

    explicit Window();

    Vec2<u32> getSize() const;
    Vec2<u32> getFramebufferSize() const;

    u32 getFramebufferWidth() const;
    u32 getFramebufferHeight() const;
    u32 getWidth() const;
    u32 getHeight() const;

    void* getHandle();
    Impl& getImpl();

private:
    void setCallbacks();
    UniquePointer<Impl> m_impl;
};

}  // namespace sl
