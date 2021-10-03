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

target_compile_features(test_set PRIVATE cxx_std_20)

set(TEST_GEN_FILE ${CMAKE_CURRENT_BINARY_DIR}/pod_struct.cpp)
add_custom_command(
	OUTPUT ${TEST_GEN_FILE}
	COMMAND compiler tests/pod_struct.h ${TEST_GEN_FILE}
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	DEPENDS compiler
)

add_executable(compiler_test
	dt_frame_main.cpp
	compiler_test.cpp
	${TEST_GEN_FILE}
)

target_link_libraries(compiler_test PRIVATE
	interpreter
	doctest
)
target_include_directories(compiler_test PRIVATE
	${CMAKE_SOURCE_DIR}
)

target_compile_features(compiler_test PRIVATE cxx_std_20)

#####################################################################

add_test (lexer_test test_set ${CMAKE_CURRENT_SOURCE_DIR}/lexis_sample.txt --dt-source-file=*lexer_test*)
add_test (interpreter_test test_set ${CMAKE_CURRENT_SOURCE_DIR}/pod_struct.txt --dt-source-file=*interpreter_test*)
add_test (compiler_test compiler_test ${CMAKE_CURRENT_SOURCE_DIR}/pod_struct.txt)

add_custom_target(run_test ALL
	COMMAND ctest -C $<CONFIG> --output-on-failure
	DEPENDS
		test_set
		compiler_test
)
