#pragma once

#include "sl/core/Window.h"

#include <gmock/gmock.h>

using namespace sl::core;

struct WindowMock : public Window {
    struct Factory : public Window::Factory {
        MOCK_METHOD(std::shared_ptr<Window>, create, (Window::Size, const std::string&), (override));
    };

    MOCK_METHOD(void, setResizeCallback, (ResizeCallback), (override));
    MOCK_METHOD(void, init, (), (override));
    MOCK_METHOD(void, update, (float), (override));
    MOCK_METHOD(bool, getShouldClose, (), (const, override));
    MOCK_METHOD(void, setShouldClose, (bool), (override));
    MOCK_METHOD(void, enableCursor, (), (override));
    MOCK_METHOD(void, disableCursor, (), (override));
    MOCK_METHOD(void*, getHandle, (), (const, override));
};