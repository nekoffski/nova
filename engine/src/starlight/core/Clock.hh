#pragma once

#include <kc/core/Clock.h>

#include <chrono>
#include <kc/core/Singleton.hpp>
#include <memory>
#include <string>

using namespace std::literals::chrono_literals;

namespace sl {

class Clock : public kc::core::Singleton<Clock> {
    using ClockImpl = kc::core::Clock;

public:
    explicit Clock();

    float getFps() const;
    float getDeltaTime() const;

    std::string formatTime(const std::string& format);

    void update();

private:
    ClockImpl m_clock;
    ClockImpl::TimePoint m_lastTimestamp;

    float m_deltaTime;
};

}  // namespace sl