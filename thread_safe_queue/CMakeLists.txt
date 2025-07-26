cmake_minimum_required(VERSION 3.30)
project(threadsafe_queue LANGUAGES CXX)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(MODULES_DIR ${SOURCE_DIR}/modules)

add_library(${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_compile_options(${PROJECT_NAME} PRIVATE -pipe -Wall -Wextra -pedantic-errors -pedantic)

target_sources(${PROJECT_NAME}
    PUBLIC
        FILE_SET modules TYPE CXX_MODULES
        BASE_DIRS ${MODULES_DIR}
        FILES
            ${MODULES_DIR}/threadsafe_queue.ixx
            ${MODULES_DIR}/threadsafe_queue_mutex.ixx
            ${MODULES_DIR}/threadsafe_std_queue.ixx
            ${MODULES_DIR}/threadsafe_queue_concept.ixx
)
