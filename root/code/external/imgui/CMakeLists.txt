file(GLOB IMGU_HPP_HEADERS CONFIGURE_DEPENDS "./include/*.hpp")

file(GLOB IMGU_H_HEADERS CONFIGURE_DEPENDS "./include/*.h")

file(GLOB IMGU_CPP_SOURCES CONFIGURE_DEPENDS "./src/*.cpp")

file(GLOB IMGU_C_SOURCES CONFIGURE_DEPENDS "./src/*.c")

message(STATUS "Module header files found: ${IMGU_H_HEADERS} ${IMGU_HPP_HEADERS} ")
message(STATUS "Module source files found: ${IMGU_CPP_SOURCES} ${IMGU_C_SOURCES}")

add_library(imgui 
	${IMGU_CPP_SOURCES}
	${IMGU_C_SOURCES}	
	${IMGU_H_HEADERS}
	${IMGU_HPP_HEADERS}
)


target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include )
target_link_libraries(imgui PUBLIC glfw)

target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)