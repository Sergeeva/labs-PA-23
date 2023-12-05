cmake_minimum_required(VERSION 3.21.1)
project(OpenCLTest)

set(CMAKE_CXX_STANDARD 23)

find_package(OpenCL REQUIRED)

add_executable(OpenCLTest main.cpp
        assert/assert_cl.h
        assert/assert_cl.cpp
        engine/Open_cl_engine.cpp
        engine/Open_cl_engine.h
        png/svgpng.hpp)

target_link_libraries(OpenCLTest OpenCL::OpenCL)