#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <glad.h>

#include "types.h"
#include "camera.h"
#include "scene.h"
#include "uicontroller.h"

namespace Graphics 
{
	extern Camera* pViewPortCamera;
	extern PerspCameraParams* pViewPortCameraParams;

	void blitToTexture(Texture texture, uint frameBufferId);
	void bindTextureToFrameBuffer(uint* pFrameBufferId, uint textureId, GLenum textarget, GLenum drawBuf, GLenum readBuf);

	void prepare(Scene* pScene);

	void render(Scene* pScene, const ViewportParams& viewportParams, const UI::SceneControlData& sceneControls, const float time);
}

#endif // !GRAPHICSH