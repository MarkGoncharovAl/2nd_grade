cmake_minimum_required(VERSION 3.13.4)
project(Main VERSION 1.0.0)

add_subdirectory(Common_libs) 

add_executable(run first.c)
target_link_libraries(run safe_ops)