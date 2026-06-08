cmake_minimum_required(VERSION 3.20)
project(OpenGLApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLFW
add_subdirectory(external/glfw)

# GLAD
add_library(glad external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# Main executable
add_executable(OpenGLApp
    src/main.cpp
    src/parser.cpp
)

# Include directories
target_include_directories(OpenGLApp PRIVATE
    include
    external/glad/include
    external/glfw/include
)

# Link libraries
target_link_libraries(OpenGLApp
    glfw
    glad
    opengl32
)