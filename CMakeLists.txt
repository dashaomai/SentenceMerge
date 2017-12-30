project(Poco-001)
cmake_minimum_required(VERSION 2.8.12)
add_definitions("-std=c++11")

include(${CMAKE_SOURCE_DIR}/build/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(poco main.cpp)
target_link_libraries(poco ${CONAN_LIBS})
