file(GLOB STB_HPP_HEADERS CONFIGURE_DEPENDS "./include/*.hpp")

file(GLOB STB_H_HEADERS CONFIGURE_DEPENDS "./include/*.h")

file(GLOB STB_CPP_SOURCES CONFIGURE_DEPENDS "./src/*.cpp")

file(GLOB STB_C_SOURCES CONFIGURE_DEPENDS "./src/*.c")

message(STATUS "Module header files found: ${STB_H_HEADERS} ${STB_HPP_HEADERS} ")
message(STATUS "Module source files found: ${STB_CPP_SOURCES} ${STB_C_SOURCES}")

add_library(stb
	${STB_CPP_SOURCES}
	${STB_C_SOURCES}	
	${STB_H_HEADERS}
	${STB_HPP_HEADERS}
)

target_include_directories(stb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
