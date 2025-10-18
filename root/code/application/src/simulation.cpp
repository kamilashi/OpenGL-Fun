#include "simulation.h"
#include "profiler.h"



inline void rotateCamera(Camera* camera, float degreeInRad, float distance, glm::vec3 lookAtTarget)
{
	float camX = sin(degreeInRad) * distance;
	float camZ = cos(degreeInRad) * distance;

	camera->viewMatrix = glm::lookAt(glm::vec3(camX, camera->position.y, camZ), lookAtTarget, glm::vec3(0.0, 1.0, 0.0));
}

void Simulation::run(Scene* pScene, const UI::SceneControlData& controls, float time)
{
	PROFILE_SCOPE("Simulation");

	float speed = 1.0f;
	float offsetScale = 0.3f;
	float offset = sin(time * speed) * offsetScale;

	pScene->jetTransform = glm::translate(pScene->jetStartTransform, glm::vec3(0.0f, offset, 0.0f));

	if (controls.orbitScene)
	{
		rotateCamera(&pScene->mainCamera, time * controls.sceneOrbitSpeed, 5, pScene->lookAtTarget);
	}
}