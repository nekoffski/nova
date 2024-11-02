file(GLOB_RECURSE SANDBOX_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
set(SANDBOX_TARGET starlight-sandbox)
set(SANDBOX_LIBS starlight-core starlight-renderer starlight-scene)

add_executable(${SANDBOX_TARGET} ${SANDBOX_SRC})
target_include_directories(${SANDBOX_TARGET} PUBLIC ${SL_INCLUDE} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${SANDBOX_TARGET} PUBLIC ${SANDBOX_LIBS})
target_compile_options(${SANDBOX_TARGET} PRIVATE ${SL_COMPILER_FLAGS})
