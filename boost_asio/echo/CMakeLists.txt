cmake_minimum_required(VERSION 3.30)
project(echo LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CLIENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/client)
set(CLIENT_SOURCE_DIR ${CLIENT_DIR}/src)
set(CLIENT_MODULES_DIR ${CLIENT_DIR}/modules)


set(SERVER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/server/)
set(SERVER_SOURCE_DIR ${SERVER_DIR}/src)
set(SERVER_MODULES_DIR ${SERVER_DIR}/modules)

find_package(Boost REQUIRED CONFIG COMPONENTS cobalt)

add_executable(tcp_client ${CLIENT_SOURCE_DIR}/main.cpp ${CLIENT_SOURCE_DIR}/tcp_client.cpp)
target_compile_features(tcp_client PRIVATE cxx_std_23)
target_compile_options(tcp_client PRIVATE -pipe -Wall -Wextra -pedantic-errors -pedantic)

target_sources(tcp_client
    PUBLIC
        FILE_SET modules TYPE CXX_MODULES
        BASE_DIRS ${CLIENT_MODULES_DIR}
        FILES
            ${CLIENT_MODULES_DIR}/tcp_client.ixx
)

target_link_libraries(tcp_client
    PRIVATE
        boost::boost)

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
