set(SL_WINDOW_LIBS stb dl glfw spdlog::spdlog glm::glm fmt::fmt Vulkan::Loader vulkan-headers::vulkan-headers starlight-event starlight-core) 
set(SL_WINDOW_TARGET starlight-window)
file(GLOB_RECURSE SL_WINDOW_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(${SL_WINDOW_TARGET} STATIC ${SL_WINDOW_SRC})
target_include_directories(${SL_WINDOW_TARGET} PRIVATE ${SL_INCLUDE})
target_link_libraries(${SL_WINDOW_TARGET} PUBLIC ${SL_WINDOW_LIBS})
target_compile_options(${SL_WINDOW_TARGET} PRIVATE ${SL_COMPILER_FLAGS})
