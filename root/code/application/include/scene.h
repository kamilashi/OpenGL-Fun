#ifndef SCENE_H
#define SCENE_H

#include "asset.h"
#include "application.h"
#include "uicontroller.h"

class Scene
{
public:
	Scene() {};
	void create(const ViewportParams& viewportParams);
	void update(const UI::SceneControlData& sceneData);
	void renderShadowCasterPass(float time);
	void renderMainPass(float time);
	uint getDebugTextureId() { return shadowMapTexture.id; }

	Camera mainCamera;
	glm::vec3 lookAtTarget;
	PerspCameraParams mainCameraParams;

	Camera mainLightCamera;

	glm::vec3 sceneBackgroundColor;

	glm::mat4 terrainTransform;
	glm::vec3 terrainColor;

	glm::vec3 lightPosition;
	glm::vec3 lightColor;
	glm::vec3 lightDirection;

	glm::mat4 jetStartTransform;
	glm::mat4 jetTransform;
	glm::vec3 jetColor;

	glm::mat4 lightSpaceMatrix;

	Model terrainCubeModel;
	Model jetModel;

	Shader defaultShader;
	Shader terrainShader;
	Shader unlitShader;
	Shader defaultDepthShader;
	Shader terrainDepthShader;

	// belongs in the graphics file ?
	Texture shadowMapTexture;

private:
	void render(Shader* pTerrainShaderVar, Shader* pDefaultShaderVar, const Camera& activeCam, float time, bool shadowCasterPass);
};

#endif