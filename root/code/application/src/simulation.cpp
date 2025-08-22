#include "simulation.h"


void Simulation::run(Scene* pScene, float time)
{
	float speed = 1.0f;
	float offsetScale = 0.3f;
	float offset = sin(time * speed) * offsetScale;

	pScene->jetTransform = glm::translate(pScene->jetStartTransform, glm::vec3(0.0f, offset, 0.0f));
}