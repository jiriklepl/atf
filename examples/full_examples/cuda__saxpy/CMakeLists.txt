cmake_minimum_required(VERSION 3.0.0)
project(cuda__saxpy)

set(CMAKE_CXX_STANDARD 14)

# CUDA
find_package(CUDAToolkit REQUIRED)

# add example executables
add_executable(cuda__saxpy cuda__saxpy.cpp)
target_link_libraries(cuda__saxpy CUDA::cudart CUDA::nvrtc)
