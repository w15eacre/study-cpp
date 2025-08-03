cmake_minimum_required(VERSION 3.30)
project(study_cpp LANGUAGES CXX)

add_subdirectory(thread_safe_queue)
add_subdirectory(thread_safe_list)
add_subdirectory(boost_asio)

enable_testing()
add_subdirectory(tests)