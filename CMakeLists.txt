cmake_minimum_required(VERSION 3.21)
project(zso_pr_1)

set(CMAKE_CXX_STANDARD 14)

add_executable(zso_pr_1 src/main.cpp src/Group.cpp src/Group.h src/Table.cpp src/Table.h)

target_link_libraries(zso_pr_1 pthread)
