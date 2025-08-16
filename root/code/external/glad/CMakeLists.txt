file(GLOB AP_HPP_HEADERS CONFIGURE_DEPENDS "./include/*.hpp")

file(GLOB AP_H_HEADERS CONFIGURE_DEPENDS "./include/*.h")

file(GLOB AP_CPP_SOURCES CONFIGURE_DEPENDS "./src/*.cpp")

file(GLOB AP_C_SOURCES CONFIGURE_DEPENDS "./src/*.c")

message(STATUS "Module header files found: ${AP_H_HEADERS} ${AP_HPP_HEADERS} ")
message(STATUS "Module source files found: ${AP_CPP_SOURCES} ${AP_C_SOURCES}")

add_library(glad
	${AP_CPP_SOURCES}
	${AP_C_SOURCES}	
	${AP_H_HEADERS}
	${AP_HPP_HEADERS}
)

target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
