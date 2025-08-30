set (TARGET_NAME "gtest_study_cpp")

set (TESTS_SOURCES
    "threadsafe_queue_test.cpp"
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

add_test(NAME ${TARGET_NAME}_list_test COMMAND ${TARGET_NAME} --gtest_filter=ThreadSafeListTypedFixture/*.*)

set(ECHO_SERVER_TEST "${CMAKE_CURRENT_SOURCE_DIR}/echo_server_test.sh")
add_test(NAME ${TARGET_NAME}_echo_server_test COMMAND bash ${ECHO_SERVER_TEST} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
