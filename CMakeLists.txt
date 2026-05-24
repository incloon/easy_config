####################################################################
#  This file is part of easy_config.                               #
#                                                                  #
#  Copyright (c) 2021-2022 Incloon                                 #
#                                                                  #
# easy_config is free software : you can redistribute it and/or    #
# modify it under the terms of the GNU Lesser General Public       #
# License as published by the Free Software Foundation, either     #
# version 3 of the License, or (at your option) any later version. #
#                                                                  #
# easy_config is distributed in the hope that it will be useful,   #
# but WITHOUT ANY WARRANTY; without even the implied warranty of   #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the      #
# GNU Lesser General Public License for more details.              #
#                                                                  #
# You should have received a copy of the GNU Lesser General Public #
# License along with easy_config.                                  #
# If not, see < https://www.gnu.org/licenses/>.                    #
####################################################################

cmake_minimum_required(VERSION 3.15)
project(easy_config VERSION 0.0.1 LANGUAGES CXX)

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
	include(GNUInstallDirs)
endif()

add_library(interpreter INTERFACE)

target_include_directories(interpreter INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_compile_features(interpreter INTERFACE cxx_std_26)

add_library(ezcfg::ezcfg ALIAS interpreter)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
	enable_testing()
	add_subdirectory(tests)
endif()
