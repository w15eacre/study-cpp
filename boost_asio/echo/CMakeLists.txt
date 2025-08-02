cmake_minimum_required(VERSION 3.30)
project(echo LANGUAGES CXX)

set(CLIENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/client/src)

set(SERVER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/server/)
set(SERVER_SOURCE_DIR ${SERVER_DIR}/src)
set(SERVER_MODULES_DIR ${SERVER_DIR}/modules)

find_package(Boost REQUIRED CONFIG COMPONENTS cobalt)

add_executable(echo_client ${CLIENT_SOURCE_DIR}/main.cpp)
target_compile_features(echo_client PRIVATE cxx_std_23)
target_compile_options(echo_client PRIVATE -pipe -Wall -Wextra -pedantic-errors -pedantic)

add_executable(echo_server ${SERVER_SOURCE_DIR}/main.cpp ${SERVER_SOURCE_DIR}/echo_sever.cpp)
target_compile_features(echo_server PRIVATE cxx_std_23)
target_compile_options(echo_server PRIVATE -pipe -Wall -Wextra -pedantic-errors -pedantic)

target_link_libraries(echo_server
    PRIVATE
        boost::boost)

target_sources(echo_server
    PUBLIC
        FILE_SET modules TYPE CXX_MODULES
        BASE_DIRS ${SERVER_MODULES_DIR}
        FILES
            ${SERVER_MODULES_DIR}/echo_server.ixx
)