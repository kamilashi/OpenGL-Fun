cmake_minimum_required(VERSION 3.20)
project(OpenGLApp LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build GLFW as a subproject, but skip its extras
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)

add_subdirectory(code/external/glad)
add_subdirectory(code/external/glfw)  
add_subdirectory(code/application)

target_include_directories(${PROJECT_NAME} PRIVATE code/external/glfw/include)