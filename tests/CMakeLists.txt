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
include(FetchContent)

FetchContent_Declare(doctest
	GIT_REPOSITORY    https://github.com/onqtam/doctest.git
	GIT_TAG           2.4.6
)

FetchContent_MakeAvailable(doctest)

#####################################################################

add_executable(test_set
	dt_frame_main.cpp
	lexer_test.cpp
	interpreter_test.cpp
)

target_link_libraries(test_set PRIVATE
	interpreter
	doctest
)

target_compile_features(test_set PRIVATE cxx_std_26)

#####################################################################

add_test (lexer_test test_set ${CMAKE_CURRENT_SOURCE_DIR}/lexis_sample.txt --dt-source-file=*lexer_test*)
add_test (interpreter_test test_set ${CMAKE_CURRENT_SOURCE_DIR}/test_struct_init.txt --dt-source-file=*interpreter_test*)

add_custom_target(run_test ALL
	COMMAND ctest --output-on-failure -C $<CONFIG>
	DEPENDS
		test_set
)
