
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "scene.h"
#include "graphics.h"
#include "assetloader.h"
#include "profiler.h"
#include "buildconfig.h"

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
	auto updateAmplitudeNErosion = [&](Shader& shader) 
	{
		glUseProgram(shader.id);
		shader.setCustomUniformF3(shader.getLoc("uAmplitudes"), (sceneData.peakAmplitudes));
		shader.setCustomUniformF3(shader.getLoc("uErosionIntensity"), (sceneData.erosionIntensities));
	};

	auto updateLacunarityNSampleOffset = [&](Shader& shader)
	{
		glUseProgram(shader.id);
		shader.setCustomUniformF2(shader.getLoc("uSampleOffset"), (sceneData.sampleOffset));
		shader.setCustomUniformF(shader.getLoc("uLacunarity"), sceneData.lacunarity);
	};

#ifdef USE_PREGEN
	#ifdef PREGEN_HEIGHT_ONLY
		updateAmplitudeNErosion(pScene->noiseGenShader);
		updateLacunarityNSampleOffset(pScene->noiseGenShader);
#else
		updateLacunarityNSampleOffset(pScene->noiseGenShader);
		updateAmplitudeNErosion(pScene->terrainShader);
		updateAmplitudeNErosion(pScene->terrainDepthShader);
	#endif
#else
	updateAmplitudeNErosion(pScene->terrainShader);
	updateLacunarityNSampleOffset(pScene->terrainShader);

	updateAmplitudeNErosion(pScene->terrainDepthShader);
	updateLacunarityNSampleOffset(pScene->terrainDepthShader);
#endif
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
	//jetStartTransform = glm::rotate(jetStartTransform, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
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

#ifdef USE_PREGEN
	#ifdef PREGEN_HEIGHT_ONLY
		const char* terrainShaderVer = "terrainheightbake";
	#else
		const char* terrainShaderVer = "terrainnoisebake";
	#endif
#else
	const char* terrainShaderVer = "terrainadhoc";
#endif

	defaultDepthShader = Shader("default", true, { "SHADOW_DEPTH_PASS" }, { "SHADOW_DEPTH_PASS" });
	terrainDepthShader = Shader(terrainShaderVer, "default", { "SHADOW_DEPTH_PASS" }, { "SHADOW_DEPTH_PASS" });

	defaultShader = Shader("default");
	terrainShader = Shader(terrainShaderVer);
	unlitShader = Shader("unlit");

#ifdef PREGEN_HEIGHT_ONLY
	noiseGenShader = Shader("heightbake");
#else
	noiseGenShader = Shader("noisebake");
#endif

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

	const int shadowMapResolution = 4096;

	shadowMapTexture = Texture(shadowMapResolution, shadowMapResolution, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);

	terrainShader.setTextureUniform(terrainShader.getLoc("shadowMap"), shadowMapTexture.id);
	defaultShader.setTextureUniform(defaultShader.getLoc("shadowMap"), shadowMapTexture.id);

#ifdef USE_PREGEN
	const int noiseResolution = 257;
	noiseGenQuadModel = Model(Mesh::Primitive::Quad);
	noiseMapFBO = ~0x0;

#ifdef PREGEN_HEIGHT_ONLY
	noiseGenTexture = Texture(noiseResolution, noiseResolution, GL_R32F, GL_RED, GL_FLOAT, GL_CLAMP_TO_EDGE);
#else
	noiseGenTexture = Texture(noiseResolution, noiseResolution, GL_RGB32F, GL_RGB, GL_FLOAT, GL_CLAMP_TO_EDGE);
#endif

	Graphics::bindTextureToFrameBuffer(&noiseMapFBO, noiseGenTexture.id, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT0);

	const int texSize[2] = { noiseGenTexture.width, noiseGenTexture.height };
	terrainShader.setTextureUniform(terrainShader.getLoc("uFbmNoiseMap"), noiseGenTexture.id);
	terrainShader.setCustomUniformI2(terrainShader.getLoc("uTextureSize"), texSize);

	terrainDepthShader.setTextureUniform(terrainDepthShader.getLoc("uFbmNoiseMap"), noiseGenTexture.id);
	terrainDepthShader.setCustomUniformI2(terrainDepthShader.getLoc("uTextureSize"), texSize);

	glUseProgram(noiseGenShader.id);
	noiseGenShader.setCustomUniformI2(noiseGenShader.getLoc("uTextureSize"), texSize);
#endif

	debugTextures[0] = &shadowMapTexture;
	debugTextures[1] = &noiseGenTexture;
	debugTextIdx = 0;
}

void Scene::update(const UI::SceneControlData& sceneData)
{
	setVec3(sceneData.mainLightPos, &lightPosition);

	updateMainLight(this);

	updateJet(this, sceneData);

	updateTerrainShaders(this, sceneData);

	debugTextIdx = sceneData.currentDebugTextureIdx;
}

void Scene::render(Shader* pTerrainShaderVar, Shader* pDefaultShaderVar, const Camera& activeCam, float time, bool shadowCasterPass = false)
{
	Shader& terrainShaderVar = *pTerrainShaderVar;
	Shader& defaultShaderVar = *pDefaultShaderVar;

	glUseProgram(terrainShaderVar.id);

	terrainShaderVar.setTransformUniforms(activeCam, terrainTransform);
	terrainShaderVar.setCustomUniformF(terrainShaderVar.uniforms.timeLoc, time);

	terrainCubeModel.draw();

	glUseProgram(defaultShaderVar.id);

	defaultShaderVar.setTransformUniforms(activeCam, jetTransform);
	defaultShaderVar.setMainColorUniform(jetColor);
	defaultShaderVar.setCustomUniformF(defaultShaderVar.uniforms.timeLoc, time);

	jetModel.draw();
}

void Scene::renderPrePass(float time)
{
	PROFILE_SCOPE("Noise Gen");
	// pre-bake noise
	Graphics::blitToTexture(noiseGenTexture, noiseMapFBO);

	glDisable(GL_DEPTH_TEST);

	glUseProgram(noiseGenShader.id);
	noiseGenShader.setCustomUniformF(noiseGenShader.uniforms.timeLoc, time);
	noiseGenQuadModel.draw();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Scene::renderShadowCasterPass(float time)
{
	PROFILE_SCOPE("Shadow Pass");

	render(&terrainDepthShader, &defaultDepthShader, mainLightCamera, time, true);
}

void Scene::renderMainPass(float time)
{
	PROFILE_SCOPE("Main Pass");

	render(&terrainShader, &defaultShader, mainCamera, time, false);
}