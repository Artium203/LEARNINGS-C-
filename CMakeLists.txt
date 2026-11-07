
cmake_minimum_required(VERSION 3.10)
project(untitled3)

set(CMAKE_CXX_STANDARD 17)

# Tell CMake to find our libraries
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)


# Add your source files
add_executable(untitled3
        main.cpp
        glad.c
)

target_include_directories(untitled3 PRIVATE
        include
        ${GLFW3_INCLUDE_DIRS}
        ${GLM_INCLUDE_DIRS}
)
# Link libraries
target_link_libraries(untitled3 PRIVATE glfw OpenGL::GL)
target_compile_definitions(untitled3 PRIVATE GL_SILENCE_DEPRECATION)

