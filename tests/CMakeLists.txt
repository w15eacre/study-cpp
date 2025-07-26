set (TARGET_NAME "gtest_study_cpp")

set (TESTS_SOURCES
    "threadsafe_queue_test.cpp"
    "threadsafe_queue_perf_test.cpp"
    "threadsafe_list_test.cpp")

add_executable(${TARGET_NAME} ${TESTS_SOURCES})
target_compile_features(${TARGET_NAME} PRIVATE cxx_std_20)

find_package(GTest REQUIRED CONFIG)

target_link_libraries(${TARGET_NAME} PRIVATE
    GTest::gtest
    GTest::gtest_main
    threadsafe_queue
    threadsafe_list)

target_compile_options(${TARGET_NAME} PRIVATE -pipe -Wall -Wextra -pedantic-errors -pedantic)

add_test(NAME ${TARGET_NAME}_queue_test COMMAND ${TARGET_NAME} --gtest_filter=ThreadSafeQueueTypedFixture/*.*)
add_test(NAME ${TARGET_NAME}_queue_perf COMMAND ${TARGET_NAME} --gtest_filter=PerformanceTests/*)

add_test(NAME ${TARGET_NAME}_list_test COMMAND ${TARGET_NAME} --gtest_filter=ThreadSafeListTypedFixture/*.*)