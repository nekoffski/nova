from conan import ConanFile


requirements = [
    "vulkan-loader/1.3.290.0",
    "vulkan-headers/1.3.290.0",
    "glfw/3.4",
    "fmt/10.2.1",
    "spdlog/1.14.1",
    "gtest/1.15.0",
    "benchmark/1.9.0",
    "boost/1.86.0",
    "nlohmann_json/3.11.3",
    "glm/cci.20230113"
]


class Recipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        for requirement in requirements:
            self.requires(requirement)
