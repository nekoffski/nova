#pragma once

#include <string>

#include <fmt/core.h>
#include <starlight/core/Log.hh>

/*
    TODO: not sure if the editor will need concurecny but thread-safety would be nice
    just in case
        - each logger could have a ring buffer
        - console would grab message from them
*/

namespace sle {

class Logger {
    enum class Severity { info, debug, warning };

public:
    explicit Logger(std::string& buffer);

    template <typename... Args>
    void info(const std::string& format, Args&&... args) {
        const auto message =
          fmt::format(fmt::runtime(format), std::forward<Args>(args)...);
        LOG_INFO("EditorLog - {}", message);
        writeLog(Severity::info, message);
    }

    template <typename... Args>
    void debug(const std::string& format, Args&&... args) {
        const auto message =
          fmt::format(fmt::runtime(format), std::forward<Args>(args)...);
        LOG_DEBUG("EditorLog - {}", message);
        writeLog(Severity::debug, message);
    }

    template <typename... Args>
    void warning(const std::string& format, Args&&... args) {
        const auto message =
          fmt::format(fmt::runtime(format), std::forward<Args>(args)...);
        LOG_WARN("EditorLog - {}", message);
        writeLog(Severity::warning, message);
    }

private:
    void writeLog(Severity severity, const std::string& message);
    std::string& m_buffer;
};

class Console {
public:
    explicit Console();

    void clear();
    Logger* getLogger();
    const std::string& getBuffer() const;

private:
    std::string m_buffer;
    Logger m_logger;
};

}  // namespace sle
