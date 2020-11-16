cmake_minimum_required(VERSION 3.2)

project(kasm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(kasm
	src/kasm.cpp
	src/common.hpp src/debug.hpp
	src/compoundInputFileStream.cpp src/compoundInputFileStream.hpp
	src/assembler.hpp src/assembler.cpp src/assembler.yy src/assembler_util.cpp
	src/binaryBuilder.hpp src/binaryBuilder.cpp
	src/compiler.hpp src/compiler.cpp src/compiler.yy src/compiler_util.cpp src/ast.cpp src/ast.hpp
	src/debugger.cpp src/debugger.hpp
	src/disassembler.hpp src/disassembler.cpp
	src/virtualMachine.hpp src/virtualMachine.cpp
	data/source.kasm
	data/source.k
)
add_custom_target(kasm_grammar ALL
	COMMAND bison assembler.yy -o assembler.cpp.re
	COMMAND re2c assembler.cpp.re -o assembler.cpp --no-debug-info
	COMMAND bison compiler.yy -o compiler.cpp.re --defines=compiler.tab.hpp
	COMMAND re2c compiler.cpp.re -o compiler.cpp --no-debug-info
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
)
add_dependencies(kasm kasm_grammar)

# add_executable(kasm src/common.hpp src/assembler.hpp src/assembler.cpp)
# add_executable(kvm src/common.hpp src/virtualMachine.hpp src/virtualMachine.cpp)
# add_executable(klang src/common.hpp src/compiler.hpp src/compiler.cpp)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT kasm)
set_target_properties(kasm PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/data"
)
