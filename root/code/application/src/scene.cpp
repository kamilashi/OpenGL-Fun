
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "scene.h"
#include "graphics.h"
#include "assetloader.h"

void Scene::create(const ViewportParams& viewportParams)
{
	sceneBackgroundColor = glm::vec3(0.15f, 0.07f, 0.5f);

	// transforms and colors
	terrainTransform = glm::mat4(1.0f);
	terrainColor = glm::vec3(0.7f, 0.1f, 0.3f);

	lightPosition = glm::vec3(2.0f, 2.0f, -1.5f);
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	lightDirection = glm::normalize(glm::vec3(glm::vec3(terrainTransform[3]) - lightPosition));
	glm::normalize(lightDirection);

	jetStartTransform = glm::mat4(1.0f);
	jetStartTransform = glm::translate(jetStartTransform, glm::vec3(0.0f, 0.7f, -0.0f));
	jetStartTransform = glm::rotate(jetStartTransform, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	jetStartTransform = glm::scale(jetStartTransform, glm::vec3(0.5f, 0.5f, 0.5f));
	jetTransform = glm::mat4(1.0f);
	jetColor = glm::vec3(0.7f, 0.7f, 0.7f);


	// cameras
	mainCamera.createPerspectiveProjection(mainCameraParams, viewportParams.width, viewportParams.height);
	mainCamera.createView(glm::vec3(3.2f, 8.0f, 3.2f));
	lookAtTarget = glm::vec3(terrainTransform[3]);
	lookAtTarget.y += 0.5;
	mainCamera.lookAt(lookAtTarget);

	float cameraOrbitRadius = mainCamera.position.length();

	float sceneBoxRadius = 5.0f;
	mainLightCamera.createOrthogonalProjection(-sceneBoxRadius, sceneBoxRadius, -sceneBoxRadius, sceneBoxRadius, 1.0f, 7.5f);
	mainLightCamera.createView(lightPosition);
	mainLightCamera.lookAt(glm::vec3(mainLightCamera.position + lightDirection));

	lightSpaceMatrix = mainLightCamera.projMatrix * mainLightCamera.viewMatrix;

	// models
	terrainCubeModel = AssetLoader::loadModel("terraincube.obj");
	terrainCubeSidesModel = AssetLoader::loadModel("terraincubesides.obj");
	jetModel = AssetLoader::loadModel("jet.obj");

	defaultDepthShader = Shader("default", true, { "SHADOW_DEPTH_PASS" }, { "SHADOW_DEPTH_PASS" });
	terrainDepthShader = Shader("terrain", "default", { "SHADOW_DEPTH_PASS" }, { "SHADOW_DEPTH_PASS" });

	defaultShader = Shader("default");
	terrainShader = Shader("terrain");
	unlitShader = Shader("unlit");

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

	shadowMapTexture = Texture(4096, 4096, GL_DEPTH_COMPONENT, GL_FLOAT);
}

void Scene::render(Shader* pTerrainShaderVar, Shader* pDefaultShaderVar, const Camera& activeCam, float time, bool shadowCasterPass = false)
{
	Shader& terrainShaderVar = *pTerrainShaderVar;
	Shader& defaultShaderVar = *pDefaultShaderVar;

	glUseProgram(terrainShaderVar.id);

	if (!shadowCasterPass)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, shadowMapTexture.id);
	}

	terrainShaderVar.setTransformUniforms(activeCam, terrainTransform);
	terrainShaderVar.setCustomUniformF(terrainShaderVar.uniforms.timeLoc, time);

	terrainCubeModel.draw();

	glUseProgram(defaultShaderVar.id);

	if (!shadowCasterPass)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, shadowMapTexture.id);
	}

	defaultShaderVar.setTransformUniforms(activeCam, terrainTransform);
	defaultShaderVar.setMainColorUniform(terrainColor);
	defaultShaderVar.setCustomUniformF(defaultShaderVar.uniforms.timeLoc, time);

	terrainCubeSidesModel.draw();

	defaultShaderVar.setTransformUniforms(activeCam, jetTransform);
	defaultShaderVar.setMainColorUniform(jetColor);
	jetModel.draw();
}

void Scene::renderShadowCasterPass(float time)
{
	render(&terrainDepthShader, &defaultDepthShader, mainLightCamera, time, true);
}

void Scene::renderMainPass(float time)
{
	render(&terrainShader, &defaultShader, mainCamera, time, false);
}