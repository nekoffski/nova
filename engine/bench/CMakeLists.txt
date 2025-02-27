find_package(benchmark REQUIRED)

file(GLOB_RECURSE BENCH_SRC *.cpp)

set(BENCH_LIBS starlight-core starlight-renderer)
set(BENCH_EXE ${PROJECT_NAME}_benchmark)

add_definitions(-DSPDLOG_ACTIVE_LEVEL=6)
add_executable(${BENCH_EXE} ${BENCH_SRC})

target_link_libraries(${BENCH_EXE} benchmark::benchmark_main ${BENCH_LIBS})
target_include_directories(${BENCH_EXE} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${SL_INCLUDE})
