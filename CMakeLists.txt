cmake_minimum_required(VERSION 3.2)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the kasm source code and call cmake from there")
endif()

if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif()

if (POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

project(kasm VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(KASM_DOCS     "Generate documentation target (Requires Sphynx)" OFF)
option(KASM_TESTS    "Generate test target"                            OFF)

if(KASM_DOCS)
	add_subdirectory(docs)
endif()

include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
configure_file(src/common.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/src/common.hpp)

add_library(kcommon STATIC
	src/debug.hpp
	src/specification.hpp src/specification.cpp
	src/common.hpp.in src/common.hpp
	src/assembler.hpp src/assembler.cpp
	src/debugger.hpp src/debugger.cpp
	src/disassembler.hpp src/disassembler.cpp
	src/virtual_machine.hpp src/virtual_machine.cpp
	src/parser.hpp src/parser.cpp
	src/lexer.hpp src/lexer.cpp
	src/token.hpp src/token.cpp
	src/node.hpp src/node.cpp
	src/program.hpp src/program.cpp
)
target_include_directories(kcommon PUBLIC src)

add_executable(kasm src/kasm.cpp )
target_link_libraries(kasm kcommon)

add_executable(kdb src/kdb.cpp)
target_link_libraries(kdb kcommon)

add_executable(kdasm src/kdasm.cpp)
target_link_libraries(kdasm kcommon)

add_executable(kvm src/kvm.cpp)
target_link_libraries(kvm kcommon)

if(KASM_TESTS)
	include(CTest)
	include(GoogleTest)
	enable_testing()

	find_package(GTest REQUIRED)

	add_executable(ktest test/ktest.cpp)
	target_link_libraries(ktest GTest::GTest GTest::Main kcommon)

	gtest_discover_tests(ktest)
endif()
