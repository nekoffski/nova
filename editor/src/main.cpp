
#include <starlight/core/Log.hh>
#include <starlight/core/Config.hh>

#include "utils/ProgramOptions.hh"
#include "Application.hh"

int main(int argc, char** argv) {
    sl::log::init("starlight-editor");

    try {
        if (auto opts = sle::ProgramOptions::parse(argc, argv); opts) {
            if (auto config = sl::Config::fromJson(opts->configPath); config) {
                sl::log::info("Config loaded successfully, starting engine");
                return sle::Application{ *config, opts->scene }.run();
            } else {
                sl::log::error("Could not load config");
                return -2;
            }
        } else {
            return -1;
        }
    } catch (const std::exception& e) {
        sl::log::error("Caught unhandled exception in main: {}", e.what());
        return -3;
    }
}
