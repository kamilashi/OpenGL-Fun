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
		float mainLightPos[3];
		float jetPosition[3];
		bool showShadowDepthMap;
		int currentPresetIdx;

		void fillPreset0()
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

			jetPosition[0] = 0.0f;
			jetPosition[1] = 0.7f;
			jetPosition[2] = 0.0f;

			showShadowDepthMap = false;
			currentPresetIdx = 0;
		}

		void fillPreset1()
		{
			lacunarity = 5.0f;

			peakAmplitudes[0] = 2.0f;
			peakAmplitudes[1] = 0.5f;
			peakAmplitudes[2] = 0.1;

			erosionIntensities[0] = 1.0f;
			erosionIntensities[1] = 0.7f;
			erosionIntensities[2] = 1.0f;

			mainLightPos[0] = -2.0f;
			mainLightPos[1] = 2.0f;
			mainLightPos[2] = 1.5f;

			sampleOffset[0] = 0.0f;
			sampleOffset[1] = 1.0f;

			jetPosition[0] = -0.7f;
			jetPosition[1] = 0.5f;
			jetPosition[2] = 0.3f;

			showShadowDepthMap = false;
			currentPresetIdx = 1;
		}

		void set(int presetIdx)
		{
			if (presetIdx == 0)
			{
				fillPreset0();
			}
			else
			{
				fillPreset1();
			}
		}
	};


	extern void showUiWidget(SceneControlData* pSceneControlData);
}

#endif