cmake_minimum_required(VERSION 4.0)
project(MandelbrotFractalGeneration)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp")

find_package(raylib CONFIG REQUIRED)

include_directories(
        ${PROJECT_SOURCE_DIR}/FractalCore
        ${PROJECT_SOURCE_DIR}/ColorMap
        ${PROJECT_SOURCE_DIR}/Renderer
)
add_executable(MandelbrotFractalGeneration
        main.cpp
        FractalCore/Fractal.cpp
        Renderer/FractalRender.cpp
        ColorMap/ColorMap.cpp)

target_link_libraries(MandelbrotFractalGeneration PRIVATE raylib)
