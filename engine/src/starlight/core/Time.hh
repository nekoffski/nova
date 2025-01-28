#pragma once

#include <chrono>
#include <memory>
#include <string>

#include "Singleton.hh"

using namespace std::literals::chrono_literals;

namespace sl {

constexpr float microsecondsInSecond = 1000.0f * 1000.0f;

using ClockType = std::chrono::steady_clock;
using TimePoint = std::chrono::time_point<ClockType>;

template <typename U, typename T>
float toSeconds(const std::chrono::duration<U, T>& duration) {
    using namespace std::chrono;
    return duration_cast<microseconds>(duration).count() / microsecondsInSecond;
}

float toSeconds(const TimePoint& timePoint);

std::string getTimeString(const std::string& format);

class Clock : public Singleton<Clock> {
public:
    explicit Clock();

    float getFps() const;
    float getDeltaTime() const;

    void update();

private:
    ClockType m_clock;
    TimePoint m_lastTimestamp;

    float m_deltaTime;
};

}  // namespace sl