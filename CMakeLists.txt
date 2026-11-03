cmake_minimum_required(VERSION 3.21)

project(FractalGeneration VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# compile_commands.json для clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(raylib CONFIG REQUIRED)
find_package(OpenMP REQUIRED)
find_package(Threads REQUIRED)

function(enable_project_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /utf-8)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

function(copy_mingw_runtime_dlls target)
    if(NOT MINGW)
        return()
    endif()

    # Копируем DLL рядом с exe, чтобы его можно было запускать вне MSYS2-терминала
    get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)

    set(runtime_dlls
            libgcc_s_seh-1.dll
            libstdc++-6.dll
            libwinpthread-1.dll
            libgomp-1.dll
            libraylib.dll
            raylib.dll
            glfw3.dll)

    if(ARGC GREATER 1)
        list(APPEND runtime_dlls ${ARGN})
    endif()

    foreach(DLL IN LISTS runtime_dlls)
        if(EXISTS "${MINGW_BIN_DIR}/${DLL}")
            add_custom_command(TARGET ${target} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            "${MINGW_BIN_DIR}/${DLL}"
                            "$<TARGET_FILE_DIR:${target}>"
                    COMMENT "Copying ${DLL}")
        endif()
    endforeach()
endfunction()

add_executable(FractalGeneration
        main.cpp
        app/application.cpp
        app/command_line.cpp
        app/headless.cpp
        fractal_core/fractal.cpp
        fractal_core/fractal_type.cpp
        fractal_core/iterations.cpp
        fractal_core/view.cpp
        fractal_core/view_snapshot.cpp
        renderer/fractal_render.cpp
        renderer/image_export_worker.cpp
        renderer/image_render.cpp
        renderer/localization.cpp
        renderer/render_worker.cpp
        renderer/settings.cpp
        color_map/color_map.cpp)

target_include_directories(FractalGeneration PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/app
        ${CMAKE_CURRENT_SOURCE_DIR}/fractal_core
        ${CMAKE_CURRENT_SOURCE_DIR}/color_map
        ${CMAKE_CURRENT_SOURCE_DIR}/renderer)

target_link_libraries(FractalGeneration PRIVATE
        raylib
        OpenMP::OpenMP_CXX
        Threads::Threads)

enable_project_warnings(FractalGeneration)

if(WIN32)
    # Иконка самого exe в Проводнике
    set(APP_ICON_RC "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.rc")
    set_source_files_properties(${APP_ICON_RC} PROPERTIES
            COMPILE_FLAGS "-I\"${CMAKE_CURRENT_SOURCE_DIR}/assets\""
            OBJECT_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.ico")
    target_sources(FractalGeneration PRIVATE ${APP_ICON_RC})
endif()

# Иконка окна загружается из assets/icon.png рядом с exe
add_custom_command(TARGET FractalGeneration POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:FractalGeneration>/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.png"
                "$<TARGET_FILE_DIR:FractalGeneration>/assets/icon.png"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/assets/IBMPlexMono-Regular.ttf"
                "$<TARGET_FILE_DIR:FractalGeneration>/assets/IBMPlexMono-Regular.ttf"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/assets/OFL.txt"
                "$<TARGET_FILE_DIR:FractalGeneration>/assets/OFL.txt"
        COMMENT "Copying assets")

copy_mingw_runtime_dlls(FractalGeneration)

include(CTest)

if(BUILD_TESTING)
    find_package(GTest CONFIG REQUIRED)

    add_executable(fractal_tests
            tests/command_line_test.cpp
            tests/fractal_test.cpp
            tests/fractal_type_test.cpp
            tests/color_map_test.cpp
            tests/export_presets_test.cpp
            tests/image_render_test.cpp
            tests/iterations_test.cpp
            tests/localization_test.cpp
            tests/settings_test.cpp
            tests/view_test.cpp
            tests/view_snapshot_test.cpp
            fractal_core/fractal.cpp
            fractal_core/fractal_type.cpp
            fractal_core/iterations.cpp
            fractal_core/view.cpp
            fractal_core/view_snapshot.cpp
            color_map/color_map.cpp
            app/command_line.cpp
            renderer/image_render.cpp
            renderer/localization.cpp
            renderer/settings.cpp)

    target_include_directories(fractal_tests PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/app
            ${CMAKE_CURRENT_SOURCE_DIR}/fractal_core
            ${CMAKE_CURRENT_SOURCE_DIR}/color_map
            ${CMAKE_CURRENT_SOURCE_DIR}/renderer)

    target_link_libraries(fractal_tests PRIVATE
            GTest::gtest_main
            raylib
            OpenMP::OpenMP_CXX)

    enable_project_warnings(fractal_tests)
    copy_mingw_runtime_dlls(fractal_tests libgtest.dll libgtest_main.dll)

    include(GoogleTest)
    gtest_discover_tests(fractal_tests DISCOVERY_MODE PRE_TEST)
endif()
