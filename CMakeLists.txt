cmake_minimum_required(VERSION 3.21)

project(MandelbrotFractalGeneration VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# compile_commands.json для clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(raylib CONFIG REQUIRED)
find_package(OpenMP REQUIRED)

add_executable(MandelbrotFractalGeneration
        main.cpp
        FractalCore/Fractal.cpp
        Renderer/FractalRender.cpp
        ColorMap/ColorMap.cpp)

target_include_directories(MandelbrotFractalGeneration PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/FractalCore
        ${CMAKE_CURRENT_SOURCE_DIR}/ColorMap
        ${CMAKE_CURRENT_SOURCE_DIR}/Renderer)

target_link_libraries(MandelbrotFractalGeneration PRIVATE
        raylib
        OpenMP::OpenMP_CXX)

if(MSVC)
    target_compile_options(MandelbrotFractalGeneration PRIVATE /W4 /utf-8)
else()
    target_compile_options(MandelbrotFractalGeneration PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(MINGW)
    # Копируем DLL рядом с exe, чтобы его можно было запускать вне MSYS2-терминала
    get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)

    set(MINGW_RUNTIME_DLLS
            libgcc_s_seh-1.dll
            libstdc++-6.dll
            libwinpthread-1.dll
            libgomp-1.dll
            libraylib.dll
            raylib.dll
            glfw3.dll)

    foreach(DLL IN LISTS MINGW_RUNTIME_DLLS)
        if(EXISTS "${MINGW_BIN_DIR}/${DLL}")
            add_custom_command(TARGET MandelbrotFractalGeneration POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            "${MINGW_BIN_DIR}/${DLL}"
                            "$<TARGET_FILE_DIR:MandelbrotFractalGeneration>"
                    COMMENT "Copying ${DLL}")
        endif()
    endforeach()
endif()
