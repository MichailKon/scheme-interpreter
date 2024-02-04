cmake_minimum_required(VERSION 3.27)
project(scheme-interpreter)

set(CMAKE_CXX_STANDARD 20)
include(sources.cmake)

target_include_directories(scheme_advanced PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})


add_executable(scheme_advanced_repl repl/main.cpp)
#target_link_libraries(test_scheme_advanced scheme_advanced)
target_link_libraries(scheme_advanced_repl scheme_advanced)
