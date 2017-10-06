cmake_minimum_required(VERSION 3.1)

project(stringpool)

add_library(string_pool INTERFACE)
target_include_directories(string_pool INTERFACE include)

add_library(string_tracker
	src/string_tracker.cpp
)
target_link_libraries(string_tracker string_pool)
