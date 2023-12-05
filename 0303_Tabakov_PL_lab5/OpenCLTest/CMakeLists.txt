cmake_minimum_required(VERSION 3.1)

project(PA_lab5)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCL REQUIRED)

SET(HEADERS "mandelbrot.h")

SET(SRC_FILES "main.cpp" "mandelbrot.cpp")

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} OpenCL::OpenCL)
