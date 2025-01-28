

#include "Time.hh"

using namespace std::literals::chrono_literals;

namespace sl {

float toSeconds(const TimePoint& timePoint) {
    using namespace std::chrono;
    return duration_cast<microseconds>(timePoint.time_since_epoch()).count()
           / microsecondsInSecond;
}

std::string getTimeString(const std::string& format) {
    auto t  = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, format.c_str());
    return oss.str();
}

Clock::Clock() : m_deltaTime(0.0f) {}

float Clock::getDeltaTime() const { return m_deltaTime; }

void Clock::update() {
    auto now        = m_clock.now();
    m_deltaTime     = toSeconds(now - m_lastTimestamp);
    m_lastTimestamp = now;
}

float Clock::getFps() const { return 1.0f / m_deltaTime; }

}  // namespace sl
