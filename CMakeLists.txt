cmake_minimum_required(VERSION 3.30)
project(study_cpp LANGUAGES CXX)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(${PROJECT_NAME} ${SOURCE_DIR}/main.cpp)
target_compile_options(${PROJECT_NAME} PUBLIC -pipe -Wall -Wextra -pedantic-errors -pedantic -stdlib=libc++)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

add_subdirectory(thread_safe_queue)

target_link_libraries(${PROJECT_NAME} PRIVATE threadsafe_queue c++)