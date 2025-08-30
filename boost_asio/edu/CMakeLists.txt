cmake_minimum_required(VERSION 3.30)
project(boost_edu LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

find_package(Boost REQUIRED CONFIG COMPONENTS cobalt)

add_executable(boost_edu ${SOURCE_DIR}/main.cpp)
target_link_libraries(boost_edu
    PRIVATE
        boost::boost)