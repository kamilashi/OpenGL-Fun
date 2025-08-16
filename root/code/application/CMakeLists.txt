file(GLOB HPP_HEADERS CONFIGURE_DEPENDS "./include/*.hpp")

file(GLOB H_HEADERS CONFIGURE_DEPENDS "./include/*.h")

file(GLOB CPP_SOURCES CONFIGURE_DEPENDS "./src/*.cpp")

message(STATUS "application header files found: ${HPP_HEADERS} ${H_HEADERS}")
message(STATUS "application source files found: ${CPP_SOURCES}")

find_package(OpenGL REQUIRED)

add_executable(${PROJECT_NAME} ${CPP_SOURCES} ${HPP_HEADERS} ${H_HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE ./include)

target_link_libraries(${PROJECT_NAME}
    PRIVATE 
        glad
        glm
        OpenGL::GL
)

if (TARGET glfw)
  target_link_libraries(OpenGLApp PRIVATE glfw)
elseif (TARGET glfw3)
  target_link_libraries(OpenGLApp PRIVATE glfw3)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        WIN32_EXECUTABLE OFF
        MACOSX_BUNDLE ON
)

