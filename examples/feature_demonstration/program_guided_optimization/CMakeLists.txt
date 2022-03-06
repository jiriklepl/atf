cmake_minimum_required(VERSION 3.0.0)
project(program_guided_optimization)

set(CMAKE_CXX_STANDARD 14)

# OpenCL
find_package(OpenCL REQUIRED)

# add example executables
add_executable(program_guided_optimization program_guided_optimization.cpp)
target_include_directories(program_guided_optimization PRIVATE ${OpenCL_INCLUDE_DIR})
target_link_libraries(program_guided_optimization ${OpenCL_LIBRARY})