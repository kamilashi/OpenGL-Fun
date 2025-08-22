cmake_minimum_required(VERSION 3.20)
project(OpenGLApp LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLFW config
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)

# Assimp config
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)       
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)      
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)            

set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)

set(ASSIMP_BUILD_OBJ_IMPORTER  ON  CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_STL_IMPORTER  ON  CACHE BOOL "" FORCE)

add_subdirectory(code/external/glad)
add_subdirectory(code/external/glfw)  
add_subdirectory(code/external/glm)   
add_subdirectory(code/external/imgui) 
add_subdirectory(code/application)

set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)   
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE) 

add_subdirectory(code/external/assimp EXCLUDE_FROM_ALL)  
target_link_libraries(${PROJECT_NAME}  PRIVATE assimp)

target_include_directories(${PROJECT_NAME} PUBLIC code/external/glfw/include)

configure_file(
  ${CMAKE_SOURCE_DIR}/cmake_config.hpp.in
  ${CMAKE_BINARY_DIR}/generated/config.hpp
  @ONLY
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/generated)

file(GLOB GEN_HPP_HEADERS CONFIGURE_DEPENDS "${CMAKE_BINARY_DIR}/generated/*.hpp")
message(STATUS "Generated header files found: ${GEN_HPP_HEADERS}")

target_sources(${PROJECT_NAME} PRIVATE ${GEN_HPP_HEADERS})