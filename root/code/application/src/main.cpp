#include <glad.h>
#include <GLFW/glfw3.h>
#include <stdexcept> 
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "types.h"
#include "shadersystem.h"
#include "camera.h"
#include "config.hpp"

const struct WindowParams
{
	 float aspectRatio = 1.5f;
};

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
	//#TODO: change this to embed shaders into the exe!!!
	char shadersDir[100];
	sprintf(shadersDir, "%s/shaders", DEFAULT_ASSET_DIR);

	ShaderSources src = ShaderSystem::loadShaderFiles(name, shadersDir);
	uint vs = compileStage(GL_VERTEX_SHADER, src.vertex, "vertex");
	uint fs = compileStage(GL_FRAGMENT_SHADER, src.fragment, "fragment");
	return linkProgram(vs, fs);
}

const struct ViewportParams
{
	int width;
	int height;

	void recalculate(int width, float aspectRatio)
	{
		this->width = width;
		height = static_cast<int>(width / aspectRatio);
	}
};

WindowParams startupParams;
ViewportParams defaultWindowParams;
PerspCameraParams cameraParams;
Camera camera;

void onWindowResize(GLFWwindow* pWin, int width, int height /*, CurrentWindowParams* pParams, Camera* pCamera*/)
{
	camera.createPerspectiveProjection(cameraParams, width, height);

	glViewport(0, 0, width, height);
}

int runWindow()
{
	GLFWwindow* window;

	if (!glfwInit())
		return -1;

	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

	const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

	defaultWindowParams.recalculate(static_cast<int>(mode->width / 2.0f), startupParams.aspectRatio);

	window = glfwCreateWindow(defaultWindowParams.width, defaultWindowParams.height, "OpenGLApp", nullptr, nullptr);

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

	float vertices[] = {
		// positions           // texture coords
		 1.f,  1.f, 0.0f,    1.0f, 1.0f,   // top right
		 1.f, -1.f, 0.0f,    1.0f, 0.0f,   // bottom right
		-1.f, -1.f, 0.0f,    0.0f, 0.0f,   // bottom left
		-1.f,  1.f, 0.0f,    0.0f, 1.0f,    // top left 

		 1.f,  1.f, 2.0f,    1.0f, 1.0f,   // top right
		 1.f, -1.f, 2.0f,    1.0f, 0.0f,   // bottom right
		-1.f, -1.f, 2.0f,    0.0f, 0.0f,   // bottom left
		-1.f,  1.f, 2.0f,    0.0f, 1.0f    // top left 
	};

	unsigned int vertexIndices[] = {
	0, 1, 2,   0, 2, 3,
	4, 6, 5,   4, 7, 6,
	0, 4, 5,   0, 5, 1,
	3, 2, 6,   3, 6, 7,
	0, 3, 7,   0, 7, 4,
	1, 5, 6,   1, 6, 2
	};

	uint VBO;
	uint EBO;
	uint shaderProgram;

	glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);

	glm::mat4 startTransform = glm::mat4(1.0f);
	glm::mat4 targetTransform = glm::mat4(1.0f);

	targetTransform = glm::rotate(startTransform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	glGenBuffers(1, &VBO);
	glBindVertexArray(VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertexIndices), vertexIndices, GL_STATIC_DRAW);

	// positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, nullptr);
	glEnableVertexAttribArray(0);

	// uvs
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	shaderProgram = loadShaderProgram("basic");

	glUseProgram(shaderProgram);
	uint transformLoc = glGetUniformLocation(shaderProgram, "uTransform");
	uint viewLoc = glGetUniformLocation(shaderProgram, "uView");
	uint projLoc = glGetUniformLocation(shaderProgram, "uProjection");

	camera = Camera();
	camera.createPerspectiveProjection( cameraParams, defaultWindowParams.width, defaultWindowParams.height);
	camera.createView(glm::vec3(5.0f, 5.0f, 5.0f));

	camera.lookAt(glm::vec3(startTransform[3]));

	glfwSetFramebufferSizeCallback(window, onWindowResize);

	while (!glfwWindowShouldClose(window))
	{
		//#TODO: move to -> rendering system

/*
		float totalTime = static_cast<float>(glfwGetTime());
		float speed = 1.0f;
		float offset = sin(totalTime * speed);

		targetTransform = glm::translate(startTransform, glm::vec3(offset, 0.0f, 0.0f));
		targetTransform = glm::rotate(startTransform, offset, glm::vec3(0.0f, 0.0f, 1.0f));
		targetTransform = glm::scale(startTransform, glm::vec3(0.1 + offset, 0.1 + offset, 1));*/

		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(targetTransform));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(camera.viewMatrix));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(camera.projMatrix));

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
