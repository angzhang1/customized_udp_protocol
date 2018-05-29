cmake_minimum_required(VERSION 3.1)

set(CMAKE_C_STANDARD 99)

add_library(common common.c)

add_executable(server_assgn1 server_assgn1.c)
target_link_libraries(server_assgn1 common)
add_executable(client_assgn1 client_assgn1.c)
target_link_libraries(client_assgn1 common)


