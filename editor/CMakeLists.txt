find_package(Boost REQUIRED)

file(GLOB_RECURSE EDITOR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
set(EDITOR_TARGET starlight-editor)
set(EDITOR_LIBS starlight-core starlight-renderer starlight-scene starlight-ui boost::boost)
add_executable(${EDITOR_TARGET} ${EDITOR_SRC})
target_include_directories(${EDITOR_TARGET} PUBLIC ${SL_INCLUDE} ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${EDITOR_TARGET} PUBLIC ${EDITOR_LIBS})
target_compile_options(${EDITOR_TARGET} PRIVATE ${SL_COMPILER_FLAGS})