find_package(fmt REQUIRED)

set(SL_UI_LIBS spdlog dl fmt::fmt  starlight-core starlight-renderer)
set(SL_UI_TARGET starlight-ui)
file(GLOB_RECURSE SL_UI_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(${SL_UI_TARGET} STATIC ${SL_UI_SRC})
target_include_directories(${SL_UI_TARGET} PRIVATE ${SL_INCLUDE})
target_link_libraries(${SL_UI_TARGET} PUBLIC ${SL_UI_LIBS})
target_compile_options(${SL_UI_TARGET} PRIVATE ${SL_COMPILER_FLAGS})
