find_package(fmt REQUIRED)

set(SL_SCENE_LIBS dl fmt::fmt starlight-core starlight-renderer)
set(SL_SCENE_TARGET starlight-scene)
file(GLOB_RECURSE SL_SCENE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(${SL_SCENE_TARGET} STATIC ${SL_SCENE_SRC})
target_include_directories(${SL_SCENE_TARGET} PRIVATE ${SL_INCLUDE})
target_link_libraries(${SL_SCENE_TARGET} PUBLIC ${SL_SCENE_LIBS})
target_compile_options(${SL_SCENE_TARGET} PRIVATE ${SL_COMPILER_FLAGS})
