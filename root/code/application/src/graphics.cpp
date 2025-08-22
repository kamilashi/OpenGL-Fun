
#include <glad.h>

#include "graphics.h"
#include "camera.h"
#include "scene.h"

namespace Graphics
{
	extern Camera* pViewPortCamera = nullptr;
	extern PerspCameraParams* pViewPortCameraParams = nullptr;


	inline void rotateCamera(Camera* camera, float degreeInRad, float distance, glm::vec3 lookAtTarget)
	{
		float camX = sin(degreeInRad) * distance;
		float camZ = cos(degreeInRad) * distance;

		camera->viewMatrix = glm::lookAt(glm::vec3(camX, camera->position.y, camZ), lookAtTarget, glm::vec3(0.0, 1.0, 0.0));
	}

	static uint depthMapFBO = ~0x0;

	static glm::mat4 debugQuadTransform;
	static Model debugQuadModel;
	static Shader debugShader;

	void bindDepthTexture(uint textureId, uint frameBufferId)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureId, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void blitToTexture(Texture texture, uint frameBufferId)
	{
		glViewport(0, 0, texture.width, texture.height);
		glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);
	}

	void prepare(Scene* pScene)
	{
		Graphics::pViewPortCamera = &pScene->mainCamera;
		Graphics::pViewPortCameraParams = &pScene->mainCameraParams;

		glClearColor(pScene->sceneBackgroundColor.r, 
					pScene->sceneBackgroundColor.g,
					pScene->sceneBackgroundColor.b, 1.0f);

		glEnable(GL_DEPTH_TEST);
		glClearDepth(1.0);
		glEnable(GL_CULL_FACE);

		debugQuadModel = Model(Mesh::Primitive::Quad);
		debugShader = Shader("debug", true, { "RENDER_IN_SCREEN_SPACE" });

		glUseProgram(debugShader.id);
		debugShader.setTransformUniforms(pScene->mainCamera, debugQuadTransform);

		glGenFramebuffers(1, &depthMapFBO);

		bindDepthTexture(pScene->shadowMapTexture.id, depthMapFBO);
	}

	void render(Scene* pScene, const ViewportParams& viewportParams, float time)
	{
		//rotateCamera(&pScene->mainCamera, time, 10, pScene->lookAtTarget);

		blitToTexture(pScene->shadowMapTexture, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		glDisable(GL_CULL_FACE);
		//glCullFace(GL_FRONT);
		pScene->renderShadowCasterPass(time);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, viewportParams.width, viewportParams.height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pScene->shadowMapTexture.id);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		pScene->renderMainPass(time);

/*
		glUseProgram(debugShader.id);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pScene->getDebugTextureId());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		debugQuadModel.draw();*/
	}
}