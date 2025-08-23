#include "uicontroller.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

extern void UI::showUiWidget(SceneControlData* pSceneControlData)
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

	if (ImGui::CollapsingHeader("Debug"))
	{
		ImGui::Checkbox("Show Shadow Depth Map", &pSceneControlData->showShadowDepthMap);
	}

	ImGui::End();
}