
#include <starlight/core/Log.hh>
#include <starlight/core/Config.hh>

#include "utils/ProgramOptions.hh"
#include "Application.hh"

int main(int argc, char** argv) {
    sl::initLogging("starlight-editor");

    try {
        if (auto opts = sle::ProgramOptions::parse(argc, argv); opts) {
            if (auto config = sl::Config::fromJson(opts->configPath); config) {
                LOG_INFO("Config loaded successfully, starting engine");
                return sle::Application{ *config, opts->scene }.start();
            } else {
                LOG_ERROR("Could not load config");
                return -2;
            }
        } else {
            return -1;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Caught unhandled exception in main: {}", e.what());
        return -3;
    }
}
