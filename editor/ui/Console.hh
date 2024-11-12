#pragma once

#include <string>

#include <fmt/core.h>

#include "starlight/core/Log.hh"

class Logger {
    enum class Severity { info, debug, warning };

public:
    explicit Logger(std::string& buffer);

    template <typename... Args>
    void info(const std::string& format, Args&&... args) {
        const auto message = fmt::format(format, std::forward<Args>(args)...);
        LOG_INFO("{}", message);
        writeLog(Severity::info, message);
    }

    template <typename... Args>
    void debug(const std::string& format, Args&&... args) {
        const auto message = fmt::format(format, std::forward<Args>(args)...);
        LOG_DEBUG("{}", message);
        writeLog(Severity::debug, message);
    }

    template <typename... Args>
    void warning(const std::string& format, Args&&... args) {
        const auto message = fmt::format(format, std::forward<Args>(args)...);
        LOG_WARN("{}", message);
        writeLog(Severity::warning, message);
    }

private:
    void writeLog(Severity severity, const std::string& message);

    std::string& m_buffer;
};

class Console {
public:
    explicit Console();

    void render();
    void clear();
    Logger* getLogger();

private:
    std::string m_buffer;
    Logger m_logger;
};
