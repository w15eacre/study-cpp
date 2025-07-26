cmake_minimum_required(VERSION 3.30)
project(study_cpp LANGUAGES CXX)

add_subdirectory(thread_safe_queue)
add_subdirectory(thread_safe_list)

enable_testing()
add_subdirectory(tests)