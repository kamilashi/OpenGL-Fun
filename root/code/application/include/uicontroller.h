#ifndef UICONTROLLER_H
#define UICONTROLLER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace UI
{
	// depends on the terrain shader implementation
	static const int octaveCount = 3;

	struct SceneControlData
	{
		float lacunarity;
		float sampleOffset[2];
		float peakAmplitudes[octaveCount];
		float erosionIntensities[octaveCount];
		float mainLightPos[octaveCount];
		bool showShadowDepthMap;


		void resetToDefault()
		{
			lacunarity = 2.0f;

			peakAmplitudes[0] = 1.0f;
			peakAmplitudes[1] = 1.8f;
			peakAmplitudes[2] = 0.5;

			erosionIntensities[0] = 0.1f;
			erosionIntensities[1] = 0.3f;
			erosionIntensities[2] = 1.5f;

			mainLightPos[0] = 2.0f;
			mainLightPos[1] = 2.0f;
			mainLightPos[2] = -1.5f;

			sampleOffset[0] = 0.0f;
			sampleOffset[1] = 0.0f;

			showShadowDepthMap = false;
		}
	};


	extern void showUiWidget(SceneControlData* pSceneControlData);
}

#endif