cmake_minimum_required(VERSION 3.5...3.14)

project(Nabbit VERSION 1.12 LANGUAGES CXX)

add_library(Nabbit INTERFACE)
target_include_directories(Nabbit INTERFACE
                           ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3 -std=c++0x")

if(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
    set(CMAKE_CILK_FLAGS "${CMAKE_CILK_FLAGS}" -fcilkplus)
    set(CMAKE_CILK_SERIALIZE_FLAG -include cilk/cilk_stub.h)
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
    set(CMAKE_CILK_FLAGS "${CMAKE_CILK_FLAGS}" -fcilkplus)
    set(CMAKE_CILK_SERIALIZE_FLAG "")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL ICC)
    set(CMAKE_CILK_SERIALIZE_FLAG "-cilk-serialize")
else()                              
    message(FATAL_ERROR "Unknown compiler id ${CMAKE_CXX_COMPILER_ID}")
endif()


enable_testing()
add_subdirectory(tests)
