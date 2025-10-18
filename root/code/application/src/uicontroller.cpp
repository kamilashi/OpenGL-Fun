#include "uicontroller.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

extern void UI::showSceneUiWidget(SceneControlData* pSceneControlData)
{
	ImGui::Begin("Scene Controller");

	if (ImGui::CollapsingHeader("Terrain Generation"))
	{
		ImGui::InputFloat("Lacunarity", &pSceneControlData->lacunarity, 1.5f, 16.0f, "%.3f");

		ImGui::InputFloat3("Peak Amplitudes", pSceneControlData->peakAmplitudes);

		ImGui::InputFloat3("Erosion Intensities", pSceneControlData->erosionIntensities);

		ImGui::InputFloat2("Sample Offset", pSceneControlData->sampleOffset);
	}

	if (ImGui::CollapsingHeader("Lighting"))
	{
		ImGui::InputFloat3("Sun Position", pSceneControlData->mainLightPos);
	}
	
	if (ImGui::CollapsingHeader("Jet"))
	{
		ImGui::InputFloat3("Root Position", pSceneControlData->jetPosition);
	}

	if (ImGui::CollapsingHeader("Simulation"))
	{
		ImGui::Checkbox("Orbit Scene", &pSceneControlData->orbitScene);
		ImGui::InputFloat("Orbit Speed", &pSceneControlData->sceneOrbitSpeed, 0.01f, 2.0f, "%.3f");
	}

	if (ImGui::CollapsingHeader("Debug"))
	{
		ImGui::Checkbox("Show Debug Map", &pSceneControlData->showShadowDepthMap);

		const char* debugTexNames[] =
		{
			"Shadow Map", "Noise Map"
		};

		ImGui::Combo("Debug Texture", &pSceneControlData->currentDebugTextureIdx, debugTexNames, IM_ARRAYSIZE(debugTexNames));
	}

	ImGui::Separator();

	const char* presetNames[] =
	{
		"Cappadocia", "Patagonia"
	};

	static int presetIdx;
	ImGui::Combo("Preset", &presetIdx, presetNames, IM_ARRAYSIZE(presetNames));

	if (pSceneControlData->currentPresetIdx != presetIdx)
	{
		pSceneControlData->set(presetIdx);
	}

/*
	if (ImGui::SmallButton("Reset to Default"))
	{
		pSceneControlData->fillPreset1();
	}*/

	ImGui::End();
}


extern void UI::showRuntimeUiWidget(RuntimeControlData* pViewportControlData)
{
	ImGui::Begin("Runtime Controller");

	ImGui::Checkbox("Pause", &pViewportControlData->isPaused);

	ImGui::End();
}