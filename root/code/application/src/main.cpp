#include <glad.h>
#include <GLFW/glfw3.h>

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

inline void rotateCamera(Camera* camera, float degreeInRad, float distance, glm::vec3 lookAtTarget)
{
	float camX = sin(degreeInRad) * distance;
	float camZ = cos(degreeInRad) * distance;

	camera->viewMatrix = glm::lookAt(glm::vec3(camX, camera->position.y, camZ), lookAtTarget, glm::vec3(0.0, 1.0, 0.0));
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

	void set(int width, int height)
	{
		this->width = width;
		this->height = height;
	}
};

WindowParams startupParams;
ViewportParams viewportParams;
PerspCameraParams cameraParams;
Camera camera;

void onWindowResize(GLFWwindow* pWin, int width, int height)
{
	if (height == 0)
	{
		return;
	}

	camera.createPerspectiveProjection(cameraParams, width, height);
	viewportParams.set(width, height);
	glViewport(0, 0, width, height);
}

int runWindow()
{
	GLFWwindow* window;

	if (!glfwInit())
		return -1;

	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

	const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

	viewportParams.recalculate(static_cast<int>(mode->width / 2.0f), startupParams.aspectRatio);

	window = glfwCreateWindow(viewportParams.width, viewportParams.height, "OpenGLApp", nullptr, nullptr);

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
	using namespace Graphics;

	glClearColor(0.15f, 0.07f, 0.5f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);
	glEnable(GL_CULL_FACE);

	// transforms and colors
	glm::mat4 terrainTransform = glm::mat4(1.0f);
	glm::vec3 terrainColor = glm::vec3(0.7f, 0.1f, 0.3f);

	glm::vec3 lightPosition = glm::vec3(2.0f, 3.0f, -2.0f);
	glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 lightDirection = glm::normalize(glm::vec3(glm::vec3(terrainTransform[3]) - lightPosition));
	glm::normalize(lightDirection);

	glm::mat4 jetStartTransform = glm::mat4(1.0f);
	jetStartTransform = glm::translate(jetStartTransform, glm::vec3(0.0f, 0.5f, 0.0f));
	jetStartTransform = glm::rotate(jetStartTransform, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 jetTransform = glm::mat4(1.0f);
	glm::vec3 jetColor = glm::vec3(0.7f, 0.7f, 0.7f);


	glm::mat4 debugQuadTransform = glm::mat4(1.0f);
	//debugQuadTransform = glm::translate(debugQuadTransform, lightPosition);
	debugQuadTransform = glm::rotate(debugQuadTransform, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	debugQuadTransform = glm::scale(debugQuadTransform, glm::vec3(10.1f, 10.1f, 10.1f));
	//debugQuadTransform = glm::scale(debugQuadTransform, glm::vec3(0.1f, 0.1f, 0.1f));

	// cameras
	camera = Camera();
	camera.createPerspectiveProjection(cameraParams, viewportParams.width, viewportParams.height);
	camera.createView(glm::vec3(4.0f, 9.0f, 4.0f));
	glm::vec3 lookAtTarget = glm::vec3(terrainTransform[3]);
	lookAtTarget.y += 0.5;
	camera.lookAt(lookAtTarget);

	float cameraOrbitRadius = camera.position.length();

	Camera mainLightCamera = Camera();
	float sceneBoxRadius = 5.0f;
	mainLightCamera.createOrthogonalProjection(-sceneBoxRadius, sceneBoxRadius, -sceneBoxRadius, sceneBoxRadius, 1.0f, 7.5f);
	mainLightCamera.createView(lightPosition);
	mainLightCamera.lookAt(glm::vec3(mainLightCamera.position + lightDirection));

	glm::mat4 lightSpaceMatrix;
	lightSpaceMatrix = mainLightCamera.projMatrix * mainLightCamera.viewMatrix;

	// models
	Model terrainCubeModel = AssetLoader::loadModel("terraincube.obj");
	Model terrainCubeSidesModel = AssetLoader::loadModel("terraincubesides.obj");
	Model jetModel = AssetLoader::loadModel("jet.obj");
	Model debugQuad = Model(Mesh::Primitive::Quad);

	// shaders
	Shader debugShader = Shader("debug", true, {"RENDER_IN_SCREEN_SPACE"});
	Shader defaultShader = Shader("default");
	Shader terrainShader = Shader("terrain");
	Shader unlitShader = Shader("unlit");

	Shader defaultDepthShader = Shader("default", true, { "SHADOW_DEPTH_PASS" }, { "SHADOW_DEPTH_PASS" });
	Shader terrainDepthShader = Shader("terrain", "default", { "SHADOW_DEPTH_PASS" }, { "SHADOW_DEPTH_PASS" });


	glUseProgram(debugShader.id);
	debugShader.setTransformUniforms(camera, debugQuadTransform);

	glUseProgram(defaultShader.id);
	defaultShader.setMainLightUniforms(lightColor, lightDirection);
	defaultShader.setCustomUniformM4(defaultShader.getLoc("uLightSpaceMatrix"), lightSpaceMatrix);

	glUseProgram(defaultDepthShader.id);
	defaultDepthShader.setMainLightUniforms(lightColor, lightDirection);

	glUseProgram(terrainShader.id);
	terrainShader.setMainLightUniforms(lightColor, lightDirection);
	terrainShader.setMainColorUniform(terrainColor);
	terrainShader.setCustomUniformM4(terrainShader.getLoc("uLightSpaceMatrix"), lightSpaceMatrix);

	glUseProgram(terrainDepthShader.id);
	terrainDepthShader.setMainLightUniforms(lightColor, lightDirection);

	glUseProgram(unlitShader.id);
	unlitShader.setMainColorUniform(lightColor);

	uint depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	Texture depthTexture = Texture(2048, 2048, GL_DEPTH_COMPONENT, GL_FLOAT);
	Graphics::bindDepthTexture(depthTexture.id, depthMapFBO);

	auto renderScene = [&](Shader* pTerrainShaderVar, Shader* pDefaultShaderVar, const Camera& activeCam, float time, bool shadowCasterPass = false)
		{
			Shader& terrainShaderVar = *pTerrainShaderVar;
			Shader& defaultShaderVar = *pDefaultShaderVar;

			glUseProgram(terrainShaderVar.id);

			if (!shadowCasterPass)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, depthTexture.id);
			}

			terrainShaderVar.setTransformUniforms(activeCam, terrainTransform);
			Shader::setCustomUniformF(terrainShaderVar.uniforms.timeLoc, time);

			terrainCubeModel.draw();

			glUseProgram(defaultShaderVar.id);

			if (!shadowCasterPass)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, depthTexture.id);
			}

			defaultShaderVar.setTransformUniforms(activeCam, terrainTransform);
			defaultShaderVar.setMainColorUniform(terrainColor);
			terrainCubeSidesModel.draw();

			float speed = 1.0f;
			float offsetScale = 0.3f;
			float offset = sin(time * speed) * offsetScale;

			jetTransform = glm::translate(jetStartTransform, glm::vec3(0.0f, offset, 0.0f));

			defaultShaderVar.setTransformUniforms(activeCam, jetTransform);
			defaultShaderVar.setMainColorUniform(jetColor);
			jetModel.draw();
		};

	while (!glfwWindowShouldClose(window))
	{
		float totalTime = static_cast<float>(glfwGetTime());

		//rotateCamera(&camera, totalTime, 10, lookAtTarget);


		Graphics::blitToTexture(depthTexture, depthMapFBO);
		glClear( GL_DEPTH_BUFFER_BIT); 

		glCullFace(GL_FRONT);
		renderScene(&terrainDepthShader, &defaultDepthShader, mainLightCamera, totalTime, true);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glViewport(0, 0, viewportParams.width, viewportParams.height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthTexture.id);

		glCullFace(GL_BACK);
		renderScene(&terrainShader, &defaultShader, camera, totalTime, false);


/*
		glUseProgram(debugShader.id);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthTexture.id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		debugQuad.draw();*/

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
