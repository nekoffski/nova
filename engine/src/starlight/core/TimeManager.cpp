

#include "TimeManager.h"

using namespace std::literals::chrono_literals;

namespace sl {

TimeManager::TimeManager() : m_deltaTime(0.0f) {}

float TimeManager::getDeltaTime() const { return m_deltaTime; }

void TimeManager::update() {
    auto now        = m_clock.now();
    m_deltaTime     = kc::core::toSeconds(now - m_lastTimestamp);
    m_lastTimestamp = now;
}

float TimeManager::getFps() const { return 1.0f / m_deltaTime; }

TimeManager::Clock* TimeManager::getClock() { return &m_clock; }

}  // namespace sl