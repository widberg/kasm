cmake_minimum_required(VERSION 3.2)

project(kasm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(kasm src/kasm.cpp src/common.hpp src/assembler.hpp src/assembler.cpp src/virtualMachine.hpp src/virtualMachine.cpp)
# add_executable(kasm src/common.hpp src/assembler.hpp src/assembler.cpp)
# add_executable(kvm src/common.hpp src/virtualMachine.hpp src/virtualMachine.cpp)
# add_executable(klang src/common.hpp src/compiler.hpp src/compiler.cpp)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT kasm)
set_target_properties(kasm PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/data")