cmake_minimum_required(VERSION 3.15)
project(easy_config VERSION 0.0.1 LANGUAGES CXX)

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
	include(GNUInstallDirs)
endif()

add_library(interpreter INTERFACE)
add_library(ezcfg ALIAS interpreter)
target_compile_features(interpreter INTERFACE cxx_std_11)

target_include_directories(interpreter INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

add_executable(compiler
	src/compiler.cpp
)
target_link_libraries(compiler PRIVATE
	interpreter
)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
	add_subdirectory(tests)
endif()