set(SL_EVENT_LIBS stb dl spdlog::spdlog fmt::fmt)
set(SL_EVENT_TARGET starlight-event)
file(GLOB_RECURSE SL_EVENT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(${SL_EVENT_TARGET} STATIC ${SL_EVENT_SRC})
target_include_directories(${SL_EVENT_TARGET} PRIVATE ${SL_INCLUDE})
target_link_libraries(${SL_EVENT_TARGET} PUBLIC ${SL_EVENT_LIBS})
target_compile_options(${SL_EVENT_TARGET} PRIVATE ${SL_COMPILER_FLAGS})
