#ifndef SIMULATION_H
#define SIMULATION_H

#include "scene.h"

namespace Simulation
{
	void run(Scene* pScene, const UI::SceneControlData& controls, float time);
}
#endif