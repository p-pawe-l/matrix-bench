cmake_minimum_required(VERSION 3.15)
project(matrix_comp CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-march=native HAS_MARCH_NATIVE)

# Optimized build flags shared by all targets
add_library(matrix_comp_flags INTERFACE)
target_compile_options(matrix_comp_flags INTERFACE -O3)
if(HAS_MARCH_NATIVE)
    target_compile_options(matrix_comp_flags INTERFACE -march=native)
endif()

add_library(matrix_comp_lib STATIC
    src/benchmark.cpp
)
target_include_directories(matrix_comp_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(matrix_comp_lib PUBLIC Threads::Threads matrix_comp_flags)

add_executable(matrix_comp main.cpp)
target_link_libraries(matrix_comp PRIVATE matrix_comp_lib)

install(TARGETS matrix_comp DESTINATION bin)
