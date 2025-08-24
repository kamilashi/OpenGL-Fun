
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "scene.h"
#include "graphics.h"
#include "assetloader.h"

void setVec3(const float source[3], glm::vec3* pDestication)
{
	pDestication->x = source[0];
	pDestication->y = source[1];
	pDestication->z = source[2];
}

void setVec3(const float source[3], float dest[3])
{
	dest[0] = source[0];
	dest[1] = source[1];
	dest[2] = source[2];
}

void setVec2(const float source[2], glm::vec2* pDestication)
{
	pDestication->x = source[0];
	pDestication->y = source[1];
}


void inline updateMainLight(Scene* pScene)
{
	pScene->lightDirection = glm::normalize(glm::vec3(glm::vec3(pScene->terrainTransform[3]) - pScene->lightPosition));

	const float distance = glm::length(pScene->lightPosition - glm::vec3(pScene->terrainTransform[3]));
	const float boundingSize = 5.0f;
	pScene->mainLightCamera.createOrthogonalProjection(-boundingSize, boundingSize, -boundingSize, boundingSize, distance - 2.2f, 4.5 + distance);
	pScene->mainLightCamera.createView(pScene->lightPosition);
	pScene->mainLightCamera.lookAt(glm::vec3(pScene->mainLightCamera.position + pScene->lightDirection));

	pScene->lightSpaceMatrix = pScene->mainLightCamera.projMatrix * pScene->mainLightCamera.viewMatrix;

	glUseProgram(pScene->defaultShader.id);
	pScene->defaultShader.setMainLightUniforms(pScene->lightColor, pScene->lightDirection);
	pScene->defaultShader.setCustomUniformM4(pScene->defaultShader.getLoc("uLightSpaceMatrix"), pScene->lightSpaceMatrix);

	glUseProgram(pScene->defaultDepthShader.id);
	pScene->defaultDepthShader.setMainLightUniforms(pScene->lightColor, pScene->lightDirection);

	glUseProgram(pScene->terrainShader.id);
	pScene->terrainShader.setMainLightUniforms(pScene->lightColor, pScene->lightDirection);
	pScene->terrainShader.setCustomUniformM4(pScene->terrainShader.getLoc("uLightSpaceMatrix"), pScene->lightSpaceMatrix);

	glUseProgram(pScene->terrainDepthShader.id);
	pScene->terrainDepthShader.setMainLightUniforms(pScene->lightColor, pScene->lightDirection);
}


void inline updateJet(Scene* pScene, const UI::SceneControlData& sceneData)
{
	pScene->jetStartTransform[3][0] = sceneData.jetPosition[0];
	pScene->jetStartTransform[3][1] = sceneData.jetPosition[1];
	pScene->jetStartTransform[3][2] = sceneData.jetPosition[2];
}

void inline updateTerrainShaders(Scene* pScene, const UI::SceneControlData& sceneData)
{
	auto updateShader = [&](Shader& terrainShader) 
	{
		glUseProgram(terrainShader.id);
		terrainShader.setCustomUniformF3(terrainShader.getLoc("uAmplitudes"), (sceneData.peakAmplitudes));
		terrainShader.setCustomUniformF3(terrainShader.getLoc("uErosionIntensity"), (sceneData.erosionIntensities));
		terrainShader.setCustomUniformF2(terrainShader.getLoc("uSampleOffset"), (sceneData.sampleOffset));
		terrainShader.setCustomUniformF(terrainShader.getLoc("uLacunarity"), sceneData.lacunarity);
	};

	updateShader(pScene->terrainShader);
	updateShader(pScene->terrainDepthShader);
}

void Scene::create(const ViewportParams& viewportParams)
{
	sceneBackgroundColor = glm::vec3(0.15f, 0.07f, 0.5f);

	// transforms and colors
	terrainTransform = glm::mat4(1.0f);
	terrainColor = glm::vec3(0.7f, 0.1f, 0.3f);

	lightPosition = glm::vec3(2.0f, 2.0f, -1.5f);
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	lightDirection = glm::normalize(glm::vec3(glm::vec3(terrainTransform[3]) - lightPosition));

	jetStartTransform = glm::mat4(1.0f);
	jetStartTransform = glm::translate(jetStartTransform, glm::vec3(0.0f, 0.7f, -0.0f));
	jetStartTransform = glm::scale(jetStartTransform, glm::vec3(0.5f, 0.5f, 0.5f));
	jetStartTransform = glm::rotate(jetStartTransform, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	jetTransform = glm::mat4(1.0f);
	jetColor = glm::vec3(0.7f, 0.7f, 0.7f);

	// cameras
	mainCamera.createPerspectiveProjection(mainCameraParams, viewportParams.width, viewportParams.height);
	mainCamera.createView(glm::vec3(3.2f, 8.0f, 3.2f));
	lookAtTarget = glm::vec3(terrainTransform[3]);
	lookAtTarget.y += 0.5;
	mainCamera.lookAt(lookAtTarget);

	const float cameraOrbitRadius = mainCamera.position.length();

	const float sceneBoxRadius = 5.0f;
	mainLightCamera.createOrthogonalProjection(-sceneBoxRadius, sceneBoxRadius, -sceneBoxRadius, sceneBoxRadius, 1.0f, 7.5f);
	mainLightCamera.createView(lightPosition);
	mainLightCamera.lookAt(glm::vec3(mainLightCamera.position + lightDirection));

	lightSpaceMatrix = mainLightCamera.projMatrix * mainLightCamera.viewMatrix;

	// models
	terrainCubeModel = AssetLoader::loadModel("terraincube.obj");
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

void Scene::update(const UI::SceneControlData& sceneData)
{
	setVec3(sceneData.mainLightPos, &lightPosition);

	updateMainLight(this);

	updateJet(this, sceneData);

	updateTerrainShaders(this, sceneData);
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

	defaultShaderVar.setTransformUniforms(activeCam, jetTransform);
	defaultShaderVar.setMainColorUniform(jetColor);
	defaultShaderVar.setCustomUniformF(defaultShaderVar.uniforms.timeLoc, time);

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