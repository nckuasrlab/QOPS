cmake_minimum_required(VERSION 3.16)
project(llvm-pass-qpgo)

find_package(LLVM REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

if(NOT ENABLE_DEBUG)
  add_definitions(-DNDEBUG)
endif()

add_subdirectory(qpgo)  # Use your pass name here.
