cmake_minimum_required(VERSION 3.15)
enable_testing()
include(FetchContent)

FetchContent_Declare(doctest
	GIT_REPOSITORY    https://github.com/onqtam/doctest.git
	GIT_TAG           2.4.6
)

FetchContent_Declare(magic_enum
	GIT_REPOSITORY    https://github.com/Neargye/magic_enum.git
	GIT_TAG           v0.7.3
)

FetchContent_MakeAvailable(doctest)
FetchContent_MakeAvailable(magic_enum)

#####################################################################

add_executable(test_set
	dt_frame_main.cpp
	lexer_test.cpp
	interpreter_test.cpp
)

target_link_libraries(test_set PRIVATE
	interpreter
	doctest
	magic_enum
)

target_compile_features(test_set PRIVATE cxx_std_20)

#####################################################################

add_test (lexer_test test_set ${CMAKE_CURRENT_SOURCE_DIR}/lexis_sample.txt --dt-source-file=lexer_test.cpp)
add_test (interpreter_test test_set ${CMAKE_CURRENT_SOURCE_DIR}/pod_struct.txt --dt-source-file=interpreter_test.cpp)
add_test (compiler_test ../compiler ${CMAKE_CURRENT_SOURCE_DIR}/pod_struct.h ${CMAKE_CURRENT_SOURCE_DIR}/pod_struct.cpp)

add_custom_target(run_test ALL
	COMMAND ctest -C $<CONFIG> --output-on-failure
	DEPENDS
		test_set
		compiler
)
