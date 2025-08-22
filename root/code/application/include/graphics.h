#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <glad.h>

#include "types.h"
#include "camera.h"
#include "scene.h"

namespace Graphics 
{
	extern Camera* pViewPortCamera;
	extern PerspCameraParams* pViewPortCameraParams;

	extern void prepare(Scene* pScene);

	extern void render(Scene* pScene, const ViewportParams& viewportParams, float time);
}

#endif // !GRAPHICSH