cmake_minimum_required(VERSION 3.1)

project(stringpool)

add_library(string_pool INTERFACE)
target_include_directories(string_pool INTERFACE include)

add_library(string_tracker STATIC
	src/string_tracker.cpp
)
target_link_libraries(string_tracker string_pool)

include(GNUInstallDirs)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS string_tracker
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
