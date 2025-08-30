set (TARGET_NAME study_cpp_bench)

set (TESTS_SOURCES
    "threadsafe_queue_perf_test.cpp")

add_executable(${TARGET_NAME} ${TESTS_SOURCES})
target_compile_features(${TARGET_NAME} PRIVATE cxx_std_20)

find_package(benchmark REQUIRED)

target_link_libraries(${TARGET_NAME} PRIVATE
    benchmark::benchmark
    threadsafe_queue
    threadsafe_list)

target_compile_options(${TARGET_NAME} PRIVATE -pipe -Wall -Wextra -pedantic-errors -pedantic)

add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME} --benchmark_repetitions=5 --benchmark_report_aggregates_only=true --benchmark_time_unit=ms)
