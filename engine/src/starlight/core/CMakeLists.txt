set(SL_CORE_LIBS stb dl spdlog::spdlog glm::glm fmt::fmt nlohmann_json::nlohmann_json)
set(SL_CORE_TARGET starlight-core)
file(GLOB_RECURSE SL_CORE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(${SL_CORE_TARGET} STATIC ${SL_CORE_SRC})
target_include_directories(${SL_CORE_TARGET} PRIVATE ${SL_INCLUDE})
target_link_libraries(${SL_CORE_TARGET} PUBLIC ${SL_CORE_LIBS})
target_compile_options(${SL_CORE_TARGET} PRIVATE ${SL_COMPILER_FLAGS})
