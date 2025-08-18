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
add_subdirectory(code/external/glm)   
add_subdirectory(code/external/stb) 
add_subdirectory(code/application)

set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)   
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE) 

add_subdirectory(code/external/assimp)  
target_link_libraries(${PROJECT_NAME}  PRIVATE assimp)

target_include_directories(${PROJECT_NAME} PRIVATE code/external/glfw/include)

configure_file(
  ${CMAKE_SOURCE_DIR}/cmake_config.hpp.in
  ${CMAKE_BINARY_DIR}/generated/config.hpp
  @ONLY
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/generated)

file(GLOB GEN_HPP_HEADERS CONFIGURE_DEPENDS "${CMAKE_BINARY_DIR}/generated/*.hpp")
message(STATUS "Generated header files found: ${GEN_HPP_HEADERS}")

target_sources(${PROJECT_NAME} PRIVATE ${GEN_HPP_HEADERS})