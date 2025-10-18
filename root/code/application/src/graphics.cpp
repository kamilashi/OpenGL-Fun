
#include <glad.h>

#include "graphics.h"
#include "camera.h"
#include "scene.h"
#include "profiler.h"
#include "buildconfig.h"

namespace Graphics
{
	extern Camera* pViewPortCamera = nullptr;
	extern PerspCameraParams* pViewPortCameraParams = nullptr;

	static uint depthMapFBO = ~0x0;

	static glm::mat4 debugQuadTransform;
	static Model debugQuadModel;
	static Shader debugShader;

	void bindTextureToFrameBuffer(uint* pFrameBufferId, uint textureId, GLenum textarget, GLenum drawBuf, GLenum readBuf)
	{
		glGenFramebuffers(1, pFrameBufferId);
		glBindFramebuffer(GL_FRAMEBUFFER, *pFrameBufferId);
		glDrawBuffer(drawBuf);
		glReadBuffer(readBuf);
		glFramebufferTexture2D(GL_FRAMEBUFFER, textarget, GL_TEXTURE_2D, textureId, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind
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

		bindTextureToFrameBuffer(&depthMapFBO, pScene->shadowMapTexture.id, GL_DEPTH_ATTACHMENT, GL_NONE, GL_NONE);
	}

	void render(Scene* pScene, const ViewportParams& viewportParams, const bool showDebugQuad, const float time)
	{
		PROFILE_SCOPE("Render");

#ifdef PREGEN_NOISE
		pScene->renderPrePass(time);
#endif

		glEnable(GL_DEPTH_TEST);
		blitToTexture(pScene->shadowMapTexture, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		glDisable(GL_CULL_FACE);
		//glCullFace(GL_FRONT);
		pScene->renderShadowCasterPass(time);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, viewportParams.width, viewportParams.height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0 + pScene->shadowMapTexture.id);
		glBindTexture(GL_TEXTURE_2D, pScene->shadowMapTexture.id);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		pScene->renderMainPass(time);

		if (showDebugQuad)
		{
			debugShader.setTextureUniform(debugShader.getLoc("uDebugTex"), pScene->getDebugTexture().id);

			debugQuadModel.draw();
		}
	}
}