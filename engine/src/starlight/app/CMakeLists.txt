set(SL_APP_LIBS stb dl starlight-core starlight-window starlight-event starlight-renderer spirv-cross::spirv-cross)
set(SL_APP_TARGET starlight-app)
file(GLOB_RECURSE SL_APP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(${SL_APP_TARGET} STATIC ${SL_APP_SRC})
target_include_directories(${SL_APP_TARGET} PRIVATE ${SL_INCLUDE})
target_link_libraries(${SL_APP_TARGET} PUBLIC ${SL_APP_LIBS})
target_compile_options(${SL_APP_TARGET} PRIVATE ${SL_COMPILER_FLAGS})
