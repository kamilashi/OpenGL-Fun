#include <glad.h>
#include <GLFW/glfw3.h>
#include <stdexcept> 
#include <iostream>

#include "types.h"
#include "shadersystem.h"
#include "config.hpp"

const struct WindowParams
{
	 float aspectRatio = 1.5f;
};

const float vertexPositions[] = {
	0.75f, 0.75f, 0.0f,
	0.75f, -0.75f, 0.0f,
	-0.75f, -0.75f, 0.0f,
	-0.75f, 0.75f, 0.0f,
}; 

unsigned int vertexIndices[] = {  
	0, 1, 3,  
	1, 2, 3  
};

uint VBO;
uint EBO;
uint shaderProgram;

static uint compileStage(GLenum type, const std::string& src, const char* label) {
	uint id = glCreateShader(type);
	const char* csrc = src.c_str();
	glShaderSource(id, 1, &csrc, nullptr);
	glCompileShader(id);

	int ok = 0;
	glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
	if (!ok) 
	{
		int len = 0; glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
		std::string log(len, '\0');
		glGetShaderInfoLog(id, len, &len, log.data());
		std::cerr << "[Shader compile error] " << label << ":\n" << log << "\n";
		glDeleteShader(id);
		throw std::runtime_error("Shader compilation failed");
	}
	return id;
}

static uint linkProgram(uint vs, uint fs) 
{
	uint prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);

	int ok = 0;
	glGetProgramiv(prog, GL_LINK_STATUS, &ok);
	if (!ok) 
	{
		int len = 0; glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
		std::string log(len, '\0');
		glGetProgramInfoLog(prog, len, &len, log.data());

		printf("Program link error \n %s\n", log.c_str());

		glDeleteProgram(prog);
		throw std::runtime_error("Program link failed");
	}
	glDetachShader(prog, vs);
	glDetachShader(prog, fs);
	glDeleteShader(vs);
	glDeleteShader(fs);

	return prog;
}

inline uint loadShaderProgram(const char* name)
{
	char shadersDir[100];
	sprintf(shadersDir, "%s/shaders", DEFAULT_ASSET_DIR);

	ShaderSources src = ShaderSystem::loadShaderFiles(name, shadersDir);
	uint vs = compileStage(GL_VERTEX_SHADER, src.vertex, "vertex");
	uint fs = compileStage(GL_FRAGMENT_SHADER, src.fragment, "fragment");
	return linkProgram(vs, fs);
}

int runWindow()
{
	GLFWwindow* window;

	if (!glfwInit())
		return -1;

	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

	const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

	WindowParams startupParams;
	int width = static_cast<int>(mode->width / 2.0f);
	int height = static_cast<int>(width / startupParams.aspectRatio);

	window = glfwCreateWindow(static_cast<int>(mode->width / 2.0f), static_cast<int>(mode->height / 2.0f), "OpenGLApp", nullptr, nullptr);

	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	// VSync
	glfwSwapInterval(1);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) 
	{
		return -1;
	}


	//#TODO: move to -> rendering system
	glGenBuffers(1, &VBO);
	glBindVertexArray(VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertexIndices), vertexIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);
	glEnableVertexAttribArray(0);

	shaderProgram = loadShaderProgram("basic");

	while (!glfwWindowShouldClose(window))
	{
		//#TODO: move to -> rendering system
		glUseProgram(shaderProgram);

		glClear(GL_COLOR_BUFFER_BIT);

		glBindVertexArray(VBO);
		glDrawElements(GL_TRIANGLES, sizeof(vertexIndices) / sizeof(int), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}


int main(int argc, char* argv[])
{
	return runWindow();
}
