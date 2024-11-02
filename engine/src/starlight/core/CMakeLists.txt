find_package(VulkanLoader REQUIRED)
find_package(VulkanHeaders REQUIRED)
find_package(glfw3 REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

set(SL_CORE_LIBS stb dl kc glfw spdlog::spdlog fmt::fmt Vulkan::Loader vulkan-headers::vulkan-headers)
set(SL_CORE_TARGET starlight-core)
file(GLOB_RECURSE SL_CORE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(${SL_CORE_TARGET} STATIC ${SL_CORE_SRC})
target_include_directories(${SL_CORE_TARGET} PRIVATE ${SL_INCLUDE})
target_link_libraries(${SL_CORE_TARGET} PUBLIC ${SL_CORE_LIBS})
target_compile_options(${SL_CORE_TARGET} PRIVATE ${SL_COMPILER_FLAGS})
