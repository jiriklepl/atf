cmake_minimum_required(VERSION 3.10)

project(ATF
    VERSION 0.0.1
    DESCRIPTION "Auto-Tuning Framework (ATF) is a generic, general-purpose auto-tuning approach that automatically finds well-performing values of performance-critical parameters (a.k.a. tuning parameters), like the sizes of tiles and numbers of threads. ATF works for programs written in arbitrary programming languages and belonging to arbitrary application domains, and it allows tuning for arbitrary objectives (e.g., high runtime performance and/or low energy consumption).")

set(CMAKE_CXX_STANDARD 14)

add_library(ATF INTERFACE)

set(ATF_DIR ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(ATF INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

install(DIRECTORY include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.hpp"
)

install(TARGETS ATF
        EXPORT ATFConfig
        ARCHIVE  DESTINATION lib
        LIBRARY  DESTINATION lib
        RUNTIME  DESTINATION bin
)

install(EXPORT ATFConfig
        DESTINATION share/ATF/cmake
)

export(TARGETS ATF
       FILE ATFConfig.cmake
)
