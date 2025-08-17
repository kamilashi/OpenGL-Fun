#include <glad.h>
#include <GLFW/glfw3.h>
#include <stdexcept> 
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "types.h"
#include "assetloader.h"
#include "camera.h"
#include "config.hpp"

const struct WindowParams
{
	 float aspectRatio = 1.5f;
};

struct ShaderUniforms
{
	uint transformLoc; 
	uint viewLoc;
	uint projLoc; 
	uint mainColorLoc;
	uint mainLightColorLoc;
	uint mainLightDirLoc;
	uint timeLoc;

	ShaderUniforms(uint shaderProgram)
	{
		initBasicUniforms(shaderProgram);
	}

	void initBasicUniforms(uint shaderProgram)
	{
		transformLoc = glGetUniformLocation(shaderProgram, "uTransform");
		viewLoc = glGetUniformLocation(shaderProgram, "uView");
		projLoc = glGetUniformLocation(shaderProgram, "uProjection");

		mainColorLoc = glGetUniformLocation(shaderProgram, "uMainColor");

		mainLightColorLoc = glGetUniformLocation(shaderProgram, "uMainLightColor");
		mainLightDirLoc = glGetUniformLocation(shaderProgram, "uMainLightDirection");

		timeLoc = glGetUniformLocation(shaderProgram, "uTime");
	}
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

inline void rotateCamera(Camera* camera, float degreeInRad, float distance, glm::vec3 lookAtTarget)
{
	float camX = sin(degreeInRad) * distance;
	float camZ = cos(degreeInRad) * distance;

	camera->viewMatrix = glm::lookAt(glm::vec3(camX, camera->position.y, camZ), lookAtTarget, glm::vec3(0.0, 1.0, 0.0));
}

inline uint loadShaderProgram(const char* name)
{
	Graphics::ShaderSources src = AssetLoader::loadShaderFiles(name);
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

void onWindowResize(GLFWwindow* pWin, int width, int height)
{
	if (height == 0)
	{
		return;
	}

	camera.createPerspectiveProjection(cameraParams, width, height);

	glViewport(0, 0, width, height);
}

void setTransformUniforms(ShaderUniforms minUniforms, Camera camera, const glm::mat4& transform)
{
	glUniformMatrix4fv(minUniforms.transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
	glUniformMatrix4fv(minUniforms.viewLoc, 1, GL_FALSE, glm::value_ptr(camera.viewMatrix));
	glUniformMatrix4fv(minUniforms.projLoc, 1, GL_FALSE, glm::value_ptr(camera.projMatrix));
}

void setMainLightUniforms(ShaderUniforms minUniforms, const glm::vec3& lightColor, const glm::vec3& lightDir)
{
	glUniform3f(minUniforms.mainLightColorLoc, lightColor.x, lightColor.y, lightColor.z);
	glUniform3f(minUniforms.mainLightDirLoc, lightDir.x, lightDir.y, lightDir.z);
}

void setMainColorUniform(ShaderUniforms minUniforms, const glm::vec3& color)
{
	glUniform3f(minUniforms.mainColorLoc, color.x, color.y, color.z);
}

void setCustomUniformF3(uint uniformLoc, const glm::vec3& color)
{
	glUniform3f(uniformLoc, color.x, color.y, color.z);
}

void setCustomUniformF(uint uniformLoc, float value)
{
	glUniform1f(uniformLoc, value);
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

	glfwSetFramebufferSizeCallback(window, onWindowResize);

	//#TODO: move to -> rendering system

	uint EBO;

	glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);

	glm::mat4 terrainTransform = glm::mat4(1.0f);
	glm::vec3 terrainColor = glm::vec3(0.7f, 0.1f, 0.3f);

	glm::mat4 lightTransform = glm::mat4(1.0f);
	lightTransform = glm::translate(lightTransform, glm::vec3(2.0f, 2.0f, -2.0f));
	lightTransform = glm::scale(lightTransform, glm::vec3(0.1f, 0.1f, 0.1f));
	glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 lightDirection = glm::vec3(terrainTransform[3] - lightTransform[3]);
	glm::normalize(lightDirection);

	glm::mat4 jetStartTransform = glm::mat4(1.0f);
	jetStartTransform = glm::translate(jetStartTransform, glm::vec3(0.0f, 0.7f, 0.0f));
	jetStartTransform = glm::rotate(jetStartTransform, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 jetTransform = glm::mat4(1.0f);
	glm::vec3 jetColor = glm::vec3(0.7f, 0.7f, 0.7f);

	Graphics::Model terrainCubeModel = AssetLoader::loadModel("terraincube.obj");
	Graphics::Model terrainCubeSidesModel = AssetLoader::loadModel("terraincubesides.obj");
	Graphics::Model jetModel = AssetLoader::loadModel("jet.obj");

	uint defaultShaderProgram = loadShaderProgram("default");
	uint terrainShaderProgram = loadShaderProgram("terrain");
	uint unlitShaderProgram = loadShaderProgram("unlit");

	ShaderUniforms defaultShaderUniforms(defaultShaderProgram);
	ShaderUniforms terrainShaderUniforms(terrainShaderProgram);
	ShaderUniforms unlitShaderUniforms(unlitShaderProgram);

	glUseProgram(defaultShaderProgram);
	setMainLightUniforms(defaultShaderUniforms, lightColor, lightDirection);

	glUseProgram(terrainShaderProgram);
	setMainLightUniforms(terrainShaderUniforms, lightColor, lightDirection);
	setMainColorUniform(terrainShaderProgram, terrainColor);

	glUseProgram(unlitShaderProgram);
	setMainColorUniform(unlitShaderUniforms, lightColor);

	camera = Camera();
	camera.createPerspectiveProjection( cameraParams, defaultWindowParams.width, defaultWindowParams.height);
	camera.createView(glm::vec3(5.0f, 9.0f, 5.0f));
	glm::vec3 lookAtTarget = glm::vec3(terrainTransform[3]);
	lookAtTarget.y += 0.5;
	float cameraOrbitRadius = camera.position.length();

	camera.lookAt(lookAtTarget);

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float totalTime = static_cast<float>(glfwGetTime());
		//rotateCamera(&camera, totalTime, 10, lookAtTarget);

		glUseProgram(terrainShaderProgram);
		setTransformUniforms(terrainShaderUniforms, camera, terrainTransform);
		setCustomUniformF(terrainShaderUniforms.timeLoc, totalTime);

		terrainCubeModel.draw();

		glUseProgram(defaultShaderProgram);
		setTransformUniforms(defaultShaderUniforms, camera, terrainTransform);
		setMainColorUniform(defaultShaderUniforms, terrainColor);
		terrainCubeSidesModel.draw();

		float speed = 1.0f;
		float offsetScale = 0.3f;
		float offset = sin(totalTime * speed) * offsetScale;

		jetTransform = glm::translate(jetStartTransform, glm::vec3(0.0f, offset, 0.0f));

		setTransformUniforms(defaultShaderUniforms, camera, jetTransform);
		setMainColorUniform(defaultShaderUniforms, jetColor);
		jetModel.draw();


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
