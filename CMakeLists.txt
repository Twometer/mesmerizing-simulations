cmake_minimum_required(VERSION 3.19)
project(mesmerizing_simulations)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(lib/glfw)

add_executable(mesmerizing_simulations
        lib/glad/src/glad.c
        lib/imgui/imgui_impl_opengl3.cpp
        lib/imgui/imgui.cpp
        lib/imgui/imgui_tables.cpp
        lib/imgui/imgui_widgets.cpp
        lib/imgui/imgui_demo.cpp
        lib/imgui/imgui_draw.cpp
        lib/imgui/imgui_impl_glfw.cpp

        src/main.cpp)

target_include_directories(mesmerizing_simulations PRIVATE lib/glm lib/glfw/include lib/glad/include lib/imgui)
target_link_libraries(mesmerizing_simulations glfw ${GLFW_LIBRARIES})