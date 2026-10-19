cmake_minimum_required(VERSION 3.15)
project(matrix_comp CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_library(matrix_comp_lib STATIC
    src/benchmark.cpp
)
target_include_directories(matrix_comp_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(matrix_comp_lib PUBLIC Threads::Threads)

add_executable(matrix_comp main.cpp)
target_link_libraries(matrix_comp PRIVATE matrix_comp_lib)

install(TARGETS matrix_comp DESTINATION bin)
