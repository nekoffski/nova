#include "ProgramOptions.hh"

#include <boost/program_options.hpp>

#include <iostream>

#include "starlight/core/Log.hh"

namespace po = boost::program_options;

namespace sle {

static void showHelp() {
    std::cout << "usage: starlight-editor --config <path-to-config>\n";
}

std::optional<ProgramOptions> ProgramOptions::parse(int argc, char** argv) {
    po::options_description optionsDescription;
    po::variables_map vm;

    optionsDescription
      .add_options()("config", po::value<std::string>(), "Path to the json config file")(
        "help", "Print help"
      );

    po::store(po::parse_command_line(argc, argv, optionsDescription), vm);
    po::notify(vm);

    if (vm.contains("help")) {
        showHelp();
        return {};
    }

    if (not vm.contains("config")) {
        std::cout << "Config field is required, use with --help to find out more\n";
        return {};
    }

    return ProgramOptions{ .configPath = vm["config"].as<std::string>() };
}

}  // namespace sle