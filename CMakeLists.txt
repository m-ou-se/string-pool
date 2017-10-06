cmake_minimum_required(VERSION 3.1)
project(stringpool)

include(GNUInstallDirs)

add_library(string_pool INTERFACE)
target_include_directories(string_pool        INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_include_directories(string_pool SYSTEM INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_FULL_INCLUDEDIR}>)

add_library(string_tracker STATIC
	src/string_tracker.cpp
)
target_link_libraries(string_tracker string_pool)

# Install libraries and header.
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS string_pool string_tracker EXPORT ${PROJECT_NAME}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Configure and install CMake files.
configure_file(config.cmake.in ${PROJECT_NAME}Config.cmake @ONLY)
set(CMAKE_INSTALL_CMAKEDIR "lib/cmake/${PROJECT_NAME}" CACHE PATH "CMake project files")
install(EXPORT ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_CMAKEDIR} NAMESPACE ${PROJECT_NAME}::)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake" DESTINATION "${CMAKE_INSTALL_CMAKEDIR}")

# Expose namespaced targets for in-tree builds too.
add_library(${PROJECT_NAME}::string_pool    ALIAS string_pool)
add_library(${PROJECT_NAME}::string_tracker ALIAS string_tracker)
