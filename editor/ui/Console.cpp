#include "Console.hh"

#include "starlight/core/Clock.hh"
#include "starlight/ui/UI.hh"

Console::Console() : m_logger(m_buffer) {}

void Console::render() {
    sl::ui::namedScope("console-text", [&]() { sl::ui::text("{}", m_buffer); });
}

void Console::clear() { m_buffer.clear(); }

Logger* Console::getLogger() { return &m_logger; }

Logger::Logger(std::string& buffer) : m_buffer(buffer) {}

void Logger::writeLog(Severity severity, const std::string& message) {
    // auto clock     = sl::Clock::get().getClock();
    // auto timestamp = clock->getTimeString("%Y-%m-%d %H:%M:%S");
    // // TODO: append in some more performance-friendly way

    // auto fullMessage = fmt::format(
    //   "{} - [{}]: {}", timestamp,
    //   severity == Severity::debug ? "dbg"
    //   : severity == Severity::info
    //     ? "inf"
    //     : "wrn",
    //   message
    // );

    // m_buffer = fmt::format("{}\n{}", fullMessage, m_buffer);
}
