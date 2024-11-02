find_package(VulkanLoader REQUIRED)
find_package(VulkanHeaders REQUIRED)
find_package(glfw3 REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

set(SL_RENDERER_LIBS stb dl kc imgui spdlog::spdlog fmt::fmt Vulkan::Loader vulkan-headers::vulkan-headers starlight-core)
set(SL_RENDERER_TARGET starlight-renderer)
file(GLOB_RECURSE SL_RENDERER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(${SL_RENDERER_TARGET} STATIC ${SL_RENDERER_SRC})
target_include_directories(${SL_RENDERER_TARGET} PRIVATE ${SL_INCLUDE})
target_link_libraries(${SL_RENDERER_TARGET} PUBLIC ${SL_RENDERER_LIBS})
target_compile_options(${SL_RENDERER_TARGET} PRIVATE ${SL_COMPILER_FLAGS})
